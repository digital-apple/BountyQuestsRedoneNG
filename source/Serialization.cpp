#include "Serialization.h"
#include "System.h"

inline const auto ReservedLocationsRecord = _byteswap_ulong('RLOC');
inline const auto ObjectivesRecord = _byteswap_ulong('OTXT');
inline const auto TrackersRecord = _byteswap_ulong('TRCR');

void Serialization::AddTracker(RE::TESGlobal* a_global, RE::BGSLocation* a_region)
{
    logs::info("Serialization::AddTracker :: Parsing tracker: '{}' with global variable: '{:x}'", a_region->GetName(), a_global->GetFormID());

    Tracker instance{a_global, a_region};
    trackers.push_back(std::make_shared<Tracker>(instance));
    logs::info("Serialization::AddTracker :: Current number of trackers: '{}'", trackers.size());
}

void Serialization::ClearTracker(RE::BGSLocation* a_region)
{
    logs::info("Serialization::ClearTracker :: Searching for tracker: '{}'", a_region->GetName());
    for (auto& tracker : trackers) {
        logs::info("Serialization::ClearTracker :: Parsing tracker: '{}'", tracker->region->GetName());
        if (tracker->region == a_region) {
            logs::info("Serialization::ClearTracker :: Tracker found!");
            tracker->global->value = 0U;
            logs::info("Serialization::ClearTracker :: Tried to clear global variable: '{:x}'. Result value: '{}'", tracker->global->GetFormID(), tracker->global->value);
            tracker->reward.clear();
            logs::info("Serialization::ClearTracker :: Tried to clear reward counter. Result value: '{}'", tracker->reward.size());
            break;
        }
    }
}

void Serialization::DeserializeObjectivesText(RE::TESQuest* a_quest, RE::BGSLocation* a_location, std::uint16_t a_index, const std::string a_text)
{
    auto quests = System::GetSingleton()->GetQuests();

    for (auto& quest : quests) {
        if (quest->quest == a_quest) {
            if (quest->location == a_location) {
                quest->objectiveIndex = a_index;
            }

            for (auto& objective : quest->quest->objectives) {
                if (objective->index == a_index) {
                    objective->displayText = a_text;
                }
            }

            if (!IsObjectiveSerialized(a_location)) {
                objectives.push_back(std::make_unique<Objective>(a_quest, a_location, a_index, a_text));
            }
        }
    }
}

auto Serialization::GetTracker(RE::BGSLocation* a_region, Util::DIFFICULTY a_difficulty) -> std::uint32_t
{
    for (auto& tracker : trackers) {
        if (tracker->region == a_region) {
            return tracker->reward[a_difficulty];
        }
    }
    return 0U;
}

bool Serialization::IsLocationReserved(RE::BGSLocation* a_location) const
{
    return std::find_if(reservedLocations.begin(), reservedLocations.end(), [a_location](RE::BGSLocation* location) { return location == a_location; }) != reservedLocations.end();
}

bool Serialization::IsObjectiveSerialized(RE::BGSLocation* a_location) const
{
    return std::find_if(objectives.begin(), objectives.end(), [a_location](std::shared_ptr<Serialization::Objective> objective) { return objective->location == a_location; }) != objectives.end();
}

bool Serialization::IsTrackerSerialized(RE::TESGlobal* a_global) const
{
    return std::find_if(trackers.begin(), trackers.end(), [a_global](std::shared_ptr<Serialization::Tracker> tracker) { return tracker->global == a_global; }) != trackers.end();
}

// Credits to Papyrus Extender by powerofthree for the string helper functions.
bool Serialization::ReadString(SKSE::SerializationInterface* a_interface, std::string& a_string) const
{
    std::size_t size = 0;

    if (!a_interface->ReadRecordData(size)) {
        return false;
    }

    a_string.reserve(size);

    if (!a_interface->ReadRecordData(a_string.data(), static_cast<std::uint32_t>(size))) {
        return false;
    }
    return true;    
}

void Serialization::ReserveLocation(RE::BGSLocation* a_location, bool a_reserve)
{
    if (a_location) {
        if (a_reserve) {
            reservedLocations.push_back(a_location);
        } else {
            reservedLocations.erase(std::remove(reservedLocations.begin(), reservedLocations.end(), a_location), reservedLocations.end());
        }
    }
}

void Serialization::SerializeObjectivesText(RE::TESQuest* a_quest, RE::BGSLocation* a_location, std::uint16_t a_index, std::string a_text)
{
    objectives.push_back(std::make_shared<Objective>(a_quest, a_location, a_index, a_text));
}

void Serialization::SetTracker(RE::BGSLocation* a_region, Util::DIFFICULTY a_difficulty, std::uint32_t a_amount)
{
    logs::info("Serialization::SetTracker :: Searching for tracker: '{}'", a_region->GetName());
    for (auto& tracker : trackers) {
        logs::info("Serialization::SetTracker :: Parsing tracker: '{}'", tracker->region->GetName());
        if (tracker->region == a_region) {
            logs::info("Serialization::SetTracker :: Tracker found!");
            tracker->global->value = 1U;
            logs::info("Serialization::SetTracker :: Tried to set global variable: '{:x}'. Result value: '{}'", tracker->global->GetFormID(), tracker->global->value);
            tracker->reward[a_difficulty] += a_amount;
            logs::info("Serialization::SetTracker :: Tried to set reward counter for difficulty: '{}' with an increase of: '{}'. Result value: '{}'", static_cast<std::uint32_t>(a_difficulty), a_amount, tracker->reward[a_difficulty]);
            break;
        }
    }
}

bool Serialization::WriteString(SKSE::SerializationInterface* a_interface, const std::string& a_string) const
{
    std::size_t size = a_string.length() + 1;
    return a_interface->WriteRecordData(size) && a_interface->WriteRecordData(a_string.data(), static_cast<std::uint32_t>(size));
}

void Serialization::OnGameLoaded(SKSE::SerializationInterface* a_interface)
{
    std::uint32_t type, size, version;

    while (a_interface->GetNextRecordInfo(type, version, size)) {
        if (type == ReservedLocationsRecord) {
            std::size_t reservedLocationsSize;
            a_interface->ReadRecordData(&reservedLocationsSize, sizeof(reservedLocationsSize));

            for (; reservedLocationsSize > 0; --reservedLocationsSize) {
                RE::FormID oldLocationFormID;
                a_interface->ReadRecordData(&oldLocationFormID, sizeof(oldLocationFormID));
                RE::FormID newLocationFormID;

                if (!a_interface->ResolveFormID(oldLocationFormID, newLocationFormID)) {
                    logs::warn("Serialization::OnGameLoaded :: Location: '{:X}' could not be found after loading the save.", oldLocationFormID);
                    continue;
                }

                auto location = RE::TESForm::LookupByID<RE::BGSLocation>(newLocationFormID);

                if (location) {
                    GetSingleton()->reservedLocations.push_back(location);
                } else {
                    logs::warn("Serialization::OnGameLoaded :: Location: '{:X}' could not be found after loading the save.", newLocationFormID);
                }
            }
        } else if (type == ObjectivesRecord) {
            std::size_t objectivesSize;
            a_interface->ReadRecordData(&objectivesSize, sizeof(objectivesSize));

            for (; objectivesSize > 0; --objectivesSize) {
                RE::FormID oldQuestFormID;
                a_interface->ReadRecordData(&oldQuestFormID, sizeof(oldQuestFormID));
                RE::FormID newQuestFormID;

                if (!a_interface->ResolveFormID(oldQuestFormID, newQuestFormID)) {
                    logs::warn("Serialization::OnGameLoaded :: Quest: '{:X}' could not be found after loading the save.", oldQuestFormID);
                    continue;
                }

                RE::FormID oldLocationFormID;
                a_interface->ReadRecordData(&oldLocationFormID, sizeof(oldLocationFormID));

                RE::FormID newLocationFormID;
                if (!a_interface->ResolveFormID(oldLocationFormID, newLocationFormID)) {
                    logs::warn("Serialization::OnGameLoaded :: Quest: '{:X}' could not be found after loading the save.", oldLocationFormID);
                    continue;
                }

                const auto quest = RE::TESForm::LookupByID<RE::TESQuest>(newQuestFormID);
                const auto location = RE::TESForm::LookupByID<RE::BGSLocation>(newLocationFormID);

                if (quest && location) {
                    std::uint16_t objectiveIndex;
                    a_interface->ReadRecordData(&objectiveIndex, sizeof(objectiveIndex));

                    std::string text;
                    if (GetSingleton()->ReadString(a_interface, text)) {
                        GetSingleton()->DeserializeObjectivesText(quest, location, objectiveIndex, text.data());
                    }
                }
            
            }
        } else if (type == TrackersRecord) {
            System::GetSingleton()->ParseTrackers();

            std::size_t trackersSize;
            a_interface->ReadRecordData(&trackersSize, sizeof(trackersSize));

            for (; trackersSize > 0; --trackersSize) {
                RE::FormID oldGlobalFormID;
                a_interface->ReadRecordData(&oldGlobalFormID, sizeof(oldGlobalFormID));

                RE::FormID newGlobalFormID;
                if (!a_interface->ResolveFormID(oldGlobalFormID, newGlobalFormID)) {
                    logs::warn("Serialization::OnGameLoaded :: Global: '{:X}' could not be found after loading the save.", oldGlobalFormID);
                    continue;
                }

                RE::FormID oldRegionFormID;
                a_interface->ReadRecordData(&oldRegionFormID, sizeof(oldRegionFormID));

                RE::FormID newRegionFormID;
                if (!a_interface->ResolveFormID(oldRegionFormID, newRegionFormID)) {
                    logs::warn("Serialization::OnGameLoaded :: Region: '{:X}' could not be found after loading the save.", oldRegionFormID);
                    continue;
                }

                std::size_t rewardSize;
                a_interface->ReadRecordData(&rewardSize, sizeof(rewardSize));

                std::unordered_map<Util::DIFFICULTY, std::uint32_t> temporary;

                for (; rewardSize > 0; --rewardSize) {
                    Util::DIFFICULTY difficulty;
                    a_interface->ReadRecordData(&difficulty, sizeof(difficulty));

                    std::uint32_t amount;
                    a_interface->ReadRecordData(&amount, sizeof(amount));

                    temporary.try_emplace(difficulty, amount);
                }

                auto global = RE::TESForm::LookupByID<RE::TESGlobal>(newGlobalFormID);
                auto region = RE::TESForm::LookupByID<RE::BGSLocation>(newRegionFormID);

                if (global && region) {
                    if (GetSingleton()->IsTrackerSerialized(global)) {
                        logs::info("Serialization::OnGameLoaded :: Tracker: '{}' with form id '{:x}' was found. Updating reward values.", region->GetName(), global->GetFormID());
                        for (auto& tracker : GetSingleton()->trackers) {
                            if (tracker->global == global) {
                                tracker->reward = temporary;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Serialization::OnGameSaved(SKSE::SerializationInterface* a_interface)
{
    std::unique_lock lock(GetSingleton()->lock);

    if (!a_interface->OpenRecord(ReservedLocationsRecord, 0)) {
        logs::warn("Unable to open record to write cosave data.");
        return;
    }

    auto reservedLocationsSize = GetSingleton()->reservedLocations.size();
    a_interface->WriteRecordData(&reservedLocationsSize, sizeof(reservedLocationsSize));

    for (auto& location : GetSingleton()->reservedLocations) {
        a_interface->WriteRecordData(&location->formID, sizeof(location->formID));
    }

    if (!a_interface->OpenRecord(ObjectivesRecord, 0)) {
        logs::warn("Unable to open record to write cosave data.");
        return;
    }

    auto objectivesSize = GetSingleton()->objectives.size();
    a_interface->WriteRecordData(&objectivesSize, sizeof(objectivesSize));

    for (auto& objective : GetSingleton()->objectives) {
        a_interface->WriteRecordData(&objective->quest->formID, sizeof(objective->quest->formID));
        a_interface->WriteRecordData(&objective->location->formID, sizeof(objective->location->formID));
        a_interface->WriteRecordData(&objective->index, sizeof(objective->index));

        GetSingleton()->WriteString(a_interface, objective->text.data());
    }

    if (!a_interface->OpenRecord(TrackersRecord, 0)) {
        logs::warn("Unable to open record to write cosave data.");
        return;
    } 

    auto trackersSize = GetSingleton()->trackers.size();
    a_interface->WriteRecordData(&trackersSize, sizeof(trackersSize));

    for (auto& tracker : GetSingleton()->trackers) {
        a_interface->WriteRecordData(&tracker->global->formID, sizeof(tracker->global->formID));
        a_interface->WriteRecordData(&tracker->region->formID, sizeof(tracker->region->formID));

        auto rewardSize = tracker->reward.size();
        a_interface->WriteRecordData(&rewardSize, sizeof(rewardSize));

        for (auto& reward : tracker->reward) {
            a_interface->WriteRecordData(&reward.first, sizeof(reward.first));
            a_interface->WriteRecordData(&reward.second, sizeof(reward.second));
        }
    }
}

void Serialization::OnRevert(SKSE::SerializationInterface*)
{
    logs::info("Serialization::OnRevert :: Reverting data.");
    std::unique_lock lock(GetSingleton()->lock);
    GetSingleton()->reservedLocations.clear();
    GetSingleton()->objectives.clear();
    GetSingleton()->trackers.clear();
}
