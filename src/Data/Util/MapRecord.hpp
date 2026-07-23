#pragma once
#include "Data/Util/BasicRecord.hpp"
#include "Data/Util/MapSerializer.hpp"

namespace Serialization {

    template <typename Value, const uint32_t uid, const uint32_t ver = 1>
    struct MapRecord {
        static constexpr auto ID = std::byteswap(uid);
        static constexpr uint32_t MaxRecordSize = 64U * 1024U * 1024U;

        MapRecord() = default;
        MapRecord(const std::unordered_map<RE::FormID, Value>& val) {
            for (const auto& [key, data] : val) {
                value.emplace(key, std::make_unique<Value>(data));
            }
        }

        Value* Find(RE::FormID key) {
            if (auto it = value.find(key); it != value.end()) {
                return it->second.get();
            }
            return nullptr;
        }

        Value* TryEmplace(RE::FormID key) {
            if (auto* data = Find(key)) {
                return data;
            }
            auto data = std::make_unique<Value>();
            auto* result = data.get();
            value.emplace(key, std::move(data));
            return result;
        }

        bool Reset(RE::FormID key) {
            if (auto it = value.find(key); it != value.end()) {
                retired.emplace_back(std::move(it->second));
                it->second = std::make_unique<Value>();
                return true;
            }
            return false;
        }

        void Clear(bool reclaimRetired = true) {
            if (reclaimRetired) {
                retired.clear();
            }
            for (auto& [key, data] : value) {
                retired.emplace_back(std::move(data));
            }
            value.clear();
        }

        template <typename Predicate>
        void EraseIf(Predicate&& shouldErase) {
            retired.clear();
            for (auto it = value.begin(); it != value.end();) {
                if (shouldErase(it->first)) {
                    retired.emplace_back(std::move(it->second));
                    it = value.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        void Load(SKSE::SerializationInterface* serializationInterface, std::uint32_t type, std::uint32_t version, uint32_t size) {
            if (type == ID) {
                logger::debug("{}: Map is being Read", Uint32ToStr(ID));
                if (version == ver) {
                    if (size < 8U || size > MaxRecordSize) {
                        logger::error("{}: Invalid map record size: {}", Uint32ToStr(ID), size);
                        return;
                    }

                    try {
                        // Read the serialized map data
                        std::vector<uint8_t> buffer(size);
                        if (serializationInterface->ReadRecordData(buffer.data(), size)) {
                            uint32_t dataVersion = 0;
                            std::unordered_map<RE::FormID, Value> loadedMap;
                            MapSerializer<RE::FormID, Value>::Deserialize(
                                loadedMap,
                                std::span(buffer.data(), size),
                                dataVersion
                            );

                            logger::debug("{}: Map Read OK! Entry count: {}", Uint32ToStr(ID), loadedMap.size());

                            // Resolve FormIDs after loading
                            std::unordered_map<RE::FormID, Value> resolvedMap;

                            auto nam = Uint32ToStr(ID);

                            for (auto& [oldFormID, data] : loadedMap) {
                                RE::FormID newFormID;
                                if (serializationInterface->ResolveFormID(oldFormID, newFormID)) {
                                    logger::trace("{} data loaded for FormID {:08X}", nam, oldFormID);
                                    if (RE::TESForm::LookupByID<RE::Actor>(newFormID)) {
                                    	resolvedMap.insert_or_assign(newFormID, std::move(data));
                                    }
                                    else {
                                        logger::warn("{} FormID {:08X} could not be found after loading the save.", nam, newFormID);
                                    }
                                }
                                else {
                                    logger::warn("{} FormID {:08X} could not be resolved. Not adding to map.", nam, oldFormID);
                                }
                            }
                            Clear(false);
                            for (auto& [formID, data] : resolvedMap) {
                                value.emplace(formID, std::make_unique<Value>(std::move(data)));
                            }
                            return;
                        }
                    }
                    catch (const std::exception& e) {
                        logger::error("{}: Map deserialization failed: {}", Uint32ToStr(ID), e.what());
                    }
                }
                logger::error("{}: Map could not be loaded!", Uint32ToStr(ID));
            }

        }

        void Save(SKSE::SerializationInterface* serializationInterface) const {
            if (value.empty()) {
                logger::debug("{}: Nothing To Save, map is empty", Uint32ToStr(ID));
                return;
            }

            logger::debug("{}: Map is being saved! Entry count: {}", Uint32ToStr(ID), value.size());
            if (serializationInterface->OpenRecord(ID, ver)) {
                try {
                    auto nam = Uint32ToStr(ID);
                    std::unordered_map<RE::FormID, Value> snapshot;
                    snapshot.reserve(value.size());
                    for (const auto& [formID, data] : value) {
                        if (data) {
                            snapshot.emplace(formID, *data);
                        }
                    }

                    auto buffer = MapSerializer<RE::FormID, Value>::Serialize(snapshot, ver);
                    if (buffer.size() > MaxRecordSize) {
                        logger::error("{}: Map is too large to save: {} Bytes", Uint32ToStr(ID), buffer.size());
                        return;
                    }
                    if (serializationInterface->WriteRecordData(buffer.data(), static_cast<uint32_t>(buffer.size()))) {
                        logger::debug("{}: Map Save OK!", Uint32ToStr(ID));

                        for (const auto& [ActorFormID, Data] : value) {
                            logger::trace("{} data serialized for Actor FormID {:08X}", nam, ActorFormID);
                        }

                        return;
                    }
                }
                catch (const std::exception& e) {
                    logger::error("{}: Map serialization failed: {}", Uint32ToStr(ID), e.what());
                }
            }
            logger::error("{}: Map could not be saved", Uint32ToStr(ID));
        }

        private:
        std::unordered_map<RE::FormID, std::unique_ptr<Value>> value;

        // Keep one retirement generation alive so concurrent readers can finish.
        // The next bulk reset or purge reclaims the previous generation.
        std::vector<std::unique_ptr<Value>> retired;
    };
}
