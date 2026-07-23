#include "Config/Config.hpp"
#include "Config/Util/SettingsOperations.hpp"

namespace {
	std::string SanitizeFileNameComponent(std::string a_value) {
		constexpr std::string_view invalidChars = "<>:\"/\\|?*";
		for (char& ch : a_value) {
			if (static_cast<unsigned char>(ch) < 32 || invalidChars.contains(ch)) {
				ch = '_';
			}
		}

		while (!a_value.empty() && (a_value.back() == '.' || a_value.back() == ' ')) {
			a_value.pop_back();
		}

		if (a_value.empty()) {
			return "Unknown";
		}

		std::string upper = a_value;
		std::ranges::transform(upper, upper.begin(), [](unsigned char ch) {
			return static_cast<char>(std::toupper(ch));
		});
		static constexpr std::array reservedNames{
			"CON"sv, "PRN"sv, "AUX"sv, "NUL"sv,
			"COM1"sv, "COM2"sv, "COM3"sv, "COM4"sv, "COM5"sv, "COM6"sv, "COM7"sv, "COM8"sv, "COM9"sv,
			"LPT1"sv, "LPT2"sv, "LPT3"sv, "LPT4"sv, "LPT5"sv, "LPT6"sv, "LPT7"sv, "LPT8"sv, "LPT9"sv,
		};
		if (std::ranges::find(reservedNames, upper) != reservedNames.end()) {
			a_value.insert(a_value.begin(), '_');
		}

		return a_value;
	}
}

namespace GTS {

    std::string Config::DebugName() {
        return "::Config";
    }

    void Config::DataReady() {
        LoadPersistentToml();
        CopyLegacySettings();
    }

    // Serialization methods using helper class
    bool Config::SerializeStructsToTOML() {
        return SettingsOperations::SerializeAllStructsToTOML(GetSingleton(), TomlData);
    }

    bool Config::DeserializeStructsFromTOML() {
        return SettingsOperations::DeserializeAllStructsFromTOML(GetSingleton(), TomlData);
    }

    void Config::ResetToDefaults() {
        SettingsOperations::ResetAllStructsToDefaults(GetSingleton());
        TomlData = toml::ordered_table();
    }

    bool Config::LoadSettings() {

        const auto Sett = &Persistent::ModSettings.value;
        if (Sett->empty()) {
            logger::info("LoadSettingsFromString(): no TOML payload, skipping load.");
            return false;
        }

        try {
            TomlData = toml::parse_str<toml::ordered_type_config>(*Sett).as_table();
        }
        catch (const toml::exception& e) {
            TomlData = toml::ordered_table();
            Sett->clear();
            logger::error("Could not Parse Persistent Mod Settings: {}", e.what());
            return false;
        }
		catch (...) {
			TomlData = toml::ordered_table();
			Sett->clear();
			logger::error("LoadSettingsFromString() -> TOML::Parse Exception Outside of TOML11's Scope");
            return false;
        }

        if (TomlData.is_empty()) {
            logger::warn("Parsed TOML is empty, skipping load.");
            return false;
        }

        if (IsSharedSettingsEnabled(TomlData)) {
            const auto saveLocalToml = TomlData;
            if (LoadSharedToml()) {
                Advanced.bShareSettingsGlobally = true;
                logger::info("Loaded shared settings instead of save-local settings");
                return true;
            }
            logger::warn("Shared settings requested but unavailable, falling back to save-local settings");
            ResetToDefaults();
            TomlData = saveLocalToml;
        }

        return DeserializeStructsFromTOML();
    }

    bool Config::SaveSettings() {

        if (!SavePersistentToml()) {
            logger::error("Failed to save persistent TOML");
            return false;
        }

        if (!SerializeStructsToTOML()) {
			logger::error("Failed to serialize settings for save");
            return false;
        }

        bool saveLocalResult = SaveTOMLToString(TomlData);
        if (!saveLocalResult) {
            logger::error("Failed to save settings into the current save");
            return false;
        }

        if (Advanced.bShareSettingsGlobally && !SaveSharedToml()) {
            logger::error("Failed to save shared settings TOML");
            return false;
        }
        return true;

    }

    // Export functionality
    bool Config::ExportSettings() {

        if (!_fileManager.EnsureExportDirectoryExists()) {
            return false;
        }

        if (!SerializeStructsToTOML()) {
            logger::error("Failed to serialize settings for export");
            return false;
        }

        const auto player = PlayerCharacter::GetSingleton();
		const std::string playerName = SanitizeFileNameComponent(player ? player->GetName() : "Unknown");
        const std::string FileName = "Export_" + playerName +  "_" + FileUtils::GetTimestamp() + ".toml";
        auto exportPath = _fileManager.GetExportPath(FileName);
        bool result = SaveTOMLToFile(TomlData, exportPath);

        if (result) {
            logger::info("Settings exported to: {}", exportPath.string());
        }
        else {
            logger::error("Failed to export settings to: {}", exportPath.string());
        }
        return result;
    }

	bool Config::LoadFromExport(const std::filesystem::path& exportPath) {

		std::error_code ec;
		if (!std::filesystem::exists(exportPath, ec) || ec) {
			logger::error("Export file does not exist: {}", exportPath.string());
            return false;
        }

        try {
            auto exportedData = toml::parse<toml::ordered_type_config>(exportPath.string());
            TomlData = std::move(exportedData).as_table();
            bool result = DeserializeStructsFromTOML();
            if (result) {
                logger::info("Settings loaded from export: {}", exportPath.string());
            }
            return result;
        }
        catch (const toml::exception& e) {
            logger::error("Could not parse export file {}: {}", exportPath.string(), e.what());
            return false;
        }
        catch (...) {
            logger::error("Unknown exception loading from export: {}", exportPath.string());
            return false;
        }
    }

    std::vector<std::filesystem::path> Config::GetExportedFiles() {
        return _fileManager.GetExportedFiles();
    }

    bool Config::DeleteExport(const std::filesystem::path& exportPath) {
        return _fileManager.DeleteExport(exportPath);
    }

    bool Config::CleanOldExports(int keepCount) {
        return _fileManager.CleanOldExports(keepCount);
    }

    void Config::CopyLegacySettings() {
        _fileManager.CopyLegacySettings(LegacyConfigFilePath);
    }

    void Config::OnGameLoaded() {
        LoadSettings();
        spdlog::set_level(spdlog::level::from_str(Advanced.sLogLevel));
    }

    bool Config::LoadPersistentToml() {
        try {
            if (!_fileManager.CheckOrCreateFile(PersistentFilePath)) {
                logger::error("Could not check or create persistent file");
                return false;
            }

            if (std::filesystem::file_size(PersistentFilePath) > 0) {
                PersistentTomlData = toml::parse<toml::ordered_type_config>(PersistentFilePath.string());

                // Load the Persistent struct from the TOML
                if (!LoadStructFromTOML(PersistentTomlData, Persistent, "Persistent")) {
                    logger::warn("Failed to load Persistent struct, using defaults");
                    Persistent = SettingsPersistent_t{};
                    return false;
                }

                logger::info("Persistent settings loaded successfully");
                return true;
            }

            // File is empty, use defaults
            logger::info("Persistent file is empty, using defaults");
            Persistent = SettingsPersistent_t{};
            return true;
            
        }
        catch (const toml::exception& e) {
            logger::error("TOML exception loading persistent settings: {}", e.what());
            Persistent = SettingsPersistent_t{};
            return false;
        }
        catch (const std::exception& e) {
            logger::error("Exception loading persistent settings: {}", e.what());
            Persistent = SettingsPersistent_t{};
            return false;
        }
    }

    bool Config::SavePersistentToml() {
        try {
            if (!_fileManager.CheckOrCreateFile(PersistentFilePath)) {
                logger::error("Could not check or create persistent file");
                return false;
            }

            PersistentTomlData = toml::ordered_table();

            // Serialize the Persistent struct into TOML
            if (!UpdateTOMLFromStruct(PersistentTomlData, Persistent, "Persistent")) {
                logger::error("Failed to serialize Persistent struct to TOML");
                return false;
            }

            // Save to file
            if (!SaveTOMLToFile(PersistentTomlData, PersistentFilePath)) {
                logger::error("Failed to save persistent TOML to file");
                return false;
            }

            logger::info("Persistent settings saved successfully");
            return true;
        }
        catch (const toml::exception& e) {
            logger::error("TOML exception saving persistent settings: {}", e.what());
            return false;
        }
        catch (const std::exception& e) {
            logger::error("Exception saving persistent settings: {}", e.what());
            return false;
        }
    }

    bool Config::LoadSharedToml() {
        try {
            if (!_fileManager.CheckOrCreateFile(SharedConfigFilePath)) {
                logger::error("Could not check or create shared settings file");
                return false;
            }

            if (std::filesystem::file_size(SharedConfigFilePath) == 0) {
                logger::warn("Shared settings file is empty");
                return false;
            }

            auto sharedData = toml::parse<toml::ordered_type_config>(SharedConfigFilePath.string());
            TomlData = std::move(sharedData).as_table();
            if (TomlData.is_empty()) {
                logger::warn("Parsed shared settings TOML is empty");
                return false;
            }

            return DeserializeStructsFromTOML();
        }
        catch (const toml::exception& e) {
            logger::error("TOML exception loading shared settings: {}", e.what());
            return false;
        }
        catch (const std::exception& e) {
            logger::error("Exception loading shared settings: {}", e.what());
            return false;
        }
        catch (...) {
            logger::error("Unknown exception loading shared settings");
            return false;
        }
    }

    bool Config::SaveSharedToml() {
        try {
            if (!_fileManager.CheckOrCreateFile(SharedConfigFilePath)) {
                logger::error("Could not check or create shared settings file");
                return false;
            }

            if (!SaveTOMLToFile(TomlData, SharedConfigFilePath)) {
                logger::error("Failed to save shared settings TOML to file");
                return false;
            }

            logger::info("Shared settings saved successfully");
            return true;
        }
        catch (const toml::exception& e) {
            logger::error("TOML exception saving shared settings: {}", e.what());
            return false;
        }
        catch (const std::exception& e) {
            logger::error("Exception saving shared settings: {}", e.what());
            return false;
        }
        catch (...) {
            logger::error("Unknown exception saving shared settings");
            return false;
        }
    }

    bool Config::IsSharedSettingsEnabled(const toml::ordered_value& a_toml) {
        try {
            const auto advanced = toml::find_or<SettingsAdvanced_t>(a_toml, "Advanced", SettingsAdvanced_t{});
            return advanced.bShareSettingsGlobally;
        }
        catch (const toml::exception& e) {
            logger::warn("Could not inspect shared settings flag from save-local TOML: {}", e.what());
            return false;
        }
        catch (const std::exception& e) {
            logger::warn("Exception inspecting shared settings flag: {}", e.what());
            return false;
        }
        catch (...) {
            logger::warn("Unknown exception inspecting shared settings flag");
            return false;
        }
    }
}
