#pragma once

#include "Config/Settings/SettingsAdvanced.hpp"
#include "Config/Settings/SettingsAI.hpp"
#include "Config/Settings/SettingsAudio.hpp"
#include "Config/Settings/SettingsBalance.hpp"
#include "Config/Settings/SettingsAutoAim.hpp"
#include "Config/Settings/SettingsCamera.hpp"
#include "Config/Settings/SettingsCollision.hpp"
#include "Config/Settings/SettingsGameplay.hpp"
#include "Config/Settings/SettingsGeneral.hpp"
#include "Config/Settings/SettingsHidden.hpp"
#include "Config/Settings/SettingsUI.hpp"

#include "Config/Util/WindowSettingsRegistry.hpp"

namespace GTS {

	class SettingsOperations : SettingsHandler {

		private:
		template<typename Config, typename Func>
		static void ForEachStaticSetting(Config& a_conf, Func&& a_func) {
			std::apply(
				[&a_func](auto&... setting) {
					(a_func(setting), ...);
				},
				std::tie(
					a_conf.Hidden,
					a_conf.Advanced,
					a_conf.General,
					a_conf.Gameplay,
					a_conf.Balance,
					a_conf.AutoAim,
					a_conf.Audio,
					a_conf.AI,
					a_conf.Camera,
					a_conf.UI,
					a_conf.Collision
				)
			);
		}

		public:
        template<typename Config>
        static bool SerializeAllStructsToTOML(Config& a_conf, toml::ordered_value& a_toml) {
            try {
                bool updateRes = true;
                a_toml = toml::ordered_table();

				ForEachStaticSetting(a_conf, [&](auto& setting) {
					updateRes &= a_conf.UpdateTOMLFromStruct(
						a_toml,
						setting,
						std::string(toml::refl::GetFriendlyName(setting))
					);
				});

                // Serialize window settings
                if (WindowSettingsRegistry::GetSingleton().HasWindowSettings()) {
                    bool windowRes = WindowSettingsRegistry::GetSingleton().SerializeAllWindowSettings(a_toml);
                    updateRes &= windowRes;
                    if (!windowRes) {
                        logger::error("One or more window settings could not be serialized to TOML");
                    }

                    // Clean up empty intermediate tables
                    WindowSettingsRegistry::RemoveEmptyTables(a_toml);
                }

                if (!updateRes) {
                    logger::error("One or more structs could not be serialized to TOML");
                }
                return updateRes;
            }
            catch (const toml::exception& e) {
                logger::error("TOML Exception during serialization: {}", e.what());
                return false;
            }
            catch (...) {
                logger::error("Unknown exception during struct serialization");
                return false;
            }
        }

        template<typename Config>
        static bool DeserializeAllStructsFromTOML(Config& a_conf, const toml::ordered_value& a_toml) {
            try {
                bool loadRes = true;

				ForEachStaticSetting(a_conf, [&](auto& setting) {
					loadRes &= a_conf.LoadStructFromTOML(
						a_toml,
						setting,
						std::string(toml::refl::GetFriendlyName(setting))
					);
				});

                // Deserialize window settings
                if (WindowSettingsRegistry::GetSingleton().HasWindowSettings()) {
                    bool windowRes = WindowSettingsRegistry::GetSingleton().DeserializeAllWindowSettings(a_toml);
                    if (!windowRes) {
                        logger::warn("Some window settings could not be deserialized");
                    }
                    loadRes &= windowRes;
                }

                if (!loadRes) {
                    logger::critical("One or more structs could not be deserialized");
                }
                return loadRes;
            }
            catch (const toml::exception& e) {
                logger::error("TOML Exception during deserialization: {}", e.what());
                return false;
            }
            catch (const std::exception& e) {
                logger::error("Exception during deserialization: {}", e.what());
                return false;
            }
            catch (...) {
                logger::error("Unknown exception during struct deserialization");
                return false;
            }
        }

        template<typename Config>
        static void ResetAllStructsToDefaults(Config& a_conf) {
			ForEachStaticSetting(a_conf, [](auto& setting) {
				setting = std::remove_cvref_t<decltype(setting)>{};
			});

            // Reset window settings
            WindowSettingsRegistry::GetSingleton().ResetAllWindowSettings();
        }
    };
}
