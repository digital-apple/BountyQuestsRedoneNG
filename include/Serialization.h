#pragma once

#include "Util.h"

class Serialization
{
public:

    enum : std::uint32_t
    {
        kVersion = 1,
        kReservedLocations = 'RLOC',
        kObjectives = 'OBJS',
        kTrackers = 'TRCS'
    };

    struct Objective
    {
        RE::TESQuest* quest;
        RE::BGSLocation* location;
        std::uint16_t index;
        std::string text;
    };

    struct Tracker
    {
        RE::TESGlobal* global;
        RE::BGSLocation* region;
        std::unordered_map<Util::DIFFICULTY, std::uint32_t> reward;
    };

    static Serialization* GetSingleton()
    {
        static Serialization singleton;
        return &singleton;
    }

    void AddTracker(RE::TESGlobal* a_global, RE::BGSLocation* a_region);
    void ClearTracker(RE::BGSLocation* a_region);
    void DeserializeObjectivesText(RE::TESQuest* a_quest, RE::BGSLocation* a_location, std::uint16_t a_index, std::string a_text);
    auto GetTrackers() const -> const std::vector<std::shared_ptr<Tracker>>&;
    bool IsLocationReserved(RE::BGSLocation* a_location) const;
    bool IsObjectiveSerialized(RE::BGSLocation* a_location) const;
    bool IsTrackerSerialized(RE::TESGlobal* a_global) const;
    bool ReadString(SKSE::SerializationInterface* a_interface, std::string& a_string) const;
    void ReserveLocation(RE::BGSLocation* a_location, bool a_reserve);
    void SerializeObjectivesText(RE::TESQuest* a_quest, RE::BGSLocation* a_location, std::uint16_t a_index, std::string a_text);
    void SetTracker(RE::BGSLocation* a_region, Util::DIFFICULTY a_difficulty, std::uint32_t a_amount);
    bool UpdateTracker(RE::TESGlobal* a_global, RE::BGSLocation* a_region, std::unordered_map<Util::DIFFICULTY, std::uint32_t>& a_reward);
    bool WriteString(SKSE::SerializationInterface* a_interface, const std::string& a_string) const;

    static bool SaveObjectives(SKSE::SerializationInterface* a_interface);
    static bool SaveLocations(SKSE::SerializationInterface* a_interface);
    static bool SaveTrackers(SKSE::SerializationInterface* a_interface);

    static bool LoadObjectives(SKSE::SerializationInterface* a_interface);
    static bool LoadLocations(SKSE::SerializationInterface* a_interface);
    static bool LoadTrackers(SKSE::SerializationInterface* a_interface);

    static void OnGameLoaded(SKSE::SerializationInterface*);
    static void OnGameSaved(SKSE::SerializationInterface*);
    static void OnRevert(SKSE::SerializationInterface*);
private:
    Serialization() = default;
    Serialization(const Serialization&) = delete;
    Serialization(Serialization&&) = delete;

    ~Serialization() = default;

    Serialization& operator=(const Serialization&) = delete;
    Serialization& operator=(Serialization&&) = delete;

    mutable std::mutex lock;
    std::vector<std::shared_ptr<Objective>> objectives;
    std::vector<RE::BGSLocation*> reservedLocations;
    std::vector<std::shared_ptr<Tracker>> trackers;
};
