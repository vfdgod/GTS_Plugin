@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
if errorlevel 1 (
    echo ERROR: Failed to switch to the project directory
    pause
    exit /b 1
)

echo Creating Package folder structure...
set "PACKAGE_ROOT=%~dp0Package"
if exist "!PACKAGE_ROOT!" (
    echo Removing old Package folder...
    rmdir /S /Q "!PACKAGE_ROOT!"
    if errorlevel 1 (
        echo ERROR: Failed to remove old Package folder
        pause
        exit /b 1
    )
)
if not exist "Package" mkdir "Package"
if not exist "Package\SKSE\Plugins" mkdir "Package\SKSE\Plugins"

echo.
echo Checking for release package folders...
set "SOURCE_FOLDER="

if exist "Distribution\Package-Release" (
    set "SOURCE_FOLDER=Distribution\Package-Release"
    echo Found: Distribution\Package-Release
) else if exist "Distribution\Package-Release-AVX" (
    set "SOURCE_FOLDER=Distribution\Package-Release-AVX"
    echo Found: Distribution\Package-Release-AVX
) else if exist "Distribution\Package-Release-AVX2" (
    set "SOURCE_FOLDER=Distribution\Package-Release-AVX2"
    echo Found: Distribution\Package-Release-AVX2
) else (
    echo ERROR: No packaged build output found in Distribution\Package-Release, Distribution\Package-Release-AVX, or Distribution\Package-Release-AVX2!
    pause
    exit /b 1
)

echo Copying contents from !SOURCE_FOLDER! to Package\SKSE\Plugins...
xcopy "!SOURCE_FOLDER!\*" "Package\SKSE\Plugins\" /E /I /Y
if errorlevel 1 (
    echo ERROR: Failed to copy plugin files
    pause
    exit /b 1
)

echo.
echo Creating Interface folder...
if not exist "Package\Interface" mkdir "Package\Interface"

echo Copying Interface files...
if exist "Distribution\Interface" (
    xcopy "Distribution\Interface\*" "Package\Interface\" /E /I /Y
    if errorlevel 1 (
        echo ERROR: Failed to copy interface files
        pause
        exit /b 1
    )
) else (
    echo WARNING: Distribution\Interface folder not found!
)

echo.
echo Creating Scripts folder structure...
if not exist "Package\Scripts" mkdir "Package\Scripts"
if not exist "Package\Scripts\Source" mkdir "Package\Scripts\Source"

echo.
echo Checking for Skyrim Special Edition installation...
set "SKYRIM_PATH="
for /f "usebackq skip=2 tokens=*" %%a in (`reg query "HKLM\SOFTWARE\WOW6432Node\Bethesda Softworks\Skyrim Special Edition" /v "Installed Path" 2^>nul`) do (
    set "LINE=%%a"
    set "SKYRIM_PATH=!LINE:*REG_SZ    =!"
)

if "!SKYRIM_PATH!"=="" (
    echo ERROR: Skyrim Special Edition registry key not found!
    pause
    exit /b 1
)

echo Found Skyrim at: !SKYRIM_PATH!

set "COMPILER=!SKYRIM_PATH!\Papyrus Compiler\PapyrusCompiler.exe"
if not exist "!COMPILER!" (
    echo ERROR: PapyrusCompiler.exe not found at: !COMPILER!
    pause
    exit /b 1
)

set "PAPYRUS_SOURCE=Distribution\PapyrusSource"
if not exist "!PAPYRUS_SOURCE!" (
    echo ERROR: !PAPYRUS_SOURCE! folder not found!
    pause
    exit /b 1
)

set "OUTPUT_PATH=%~dp0Package\Scripts"
set "INPUT_PATH=%~dp0!PAPYRUS_SOURCE!"

rem Find the correct Skyrim scripts source folder
set "SKYRIM_SCRIPTS="
if exist "!SKYRIM_PATH!\Data\Scripts\Source" (
    set "SKYRIM_SCRIPTS=!SKYRIM_PATH!\Data\Scripts\Source"
    echo Using Skyrim scripts from: Data\Scripts\Source
) else if exist "!SKYRIM_PATH!\Data\Source\Scripts" (
    set "SKYRIM_SCRIPTS=!SKYRIM_PATH!\Data\Source\Scripts"
    echo Using Skyrim scripts from: Data\Source\Scripts
) else if exist "!SKYRIM_PATH!\Data\Scripts" (
    set "SKYRIM_SCRIPTS=!SKYRIM_PATH!\Data\Scripts"
    echo Using Skyrim scripts from: Data\Scripts
) else (
    echo ERROR: Could not find Skyrim scripts source folder!
    pause
    exit /b 1
)

echo.
echo Compiling Papyrus scripts...
set "PAPYRUS_COMPILE_FAILED="
for %%f in ("!PAPYRUS_SOURCE!\*.psc") do (
    echo Compiling: %%~nxf
    "!COMPILER!" "%%f" -f="TESV_Papyrus_Flags.flg" -i="!SKYRIM_SCRIPTS!;!INPUT_PATH!" -o="!OUTPUT_PATH!"
    if errorlevel 1 (
        echo ERROR: Failed to compile %%~nxf
        set "PAPYRUS_COMPILE_FAILED=1"
    )
)

if defined PAPYRUS_COMPILE_FAILED (
    echo ERROR: One or more Papyrus scripts failed to compile
    pause
    exit /b 1
)

echo.
echo Copying Papyrus source files...
if exist "!PAPYRUS_SOURCE!" (
    xcopy "!PAPYRUS_SOURCE!\*" "Package\Scripts\Source\" /E /I /Y
    if errorlevel 1 (
        echo ERROR: Failed to copy source files
        pause
        exit /b 1
    )
) else (
    echo ERROR: !PAPYRUS_SOURCE! folder not found!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Package creation complete!
echo ========================================
pause
exit /b 0
