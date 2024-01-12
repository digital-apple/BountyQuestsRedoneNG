#pragma once

#include "Util.h"

class System
{
public:
    struct Quest
    {
        std::string name;
        Util::DIFFICULTY difficulty;
        RE::BGSLocation* location;
        RE::BGSLocation* region;
        RE::TESQuest* quest;
        Util::TYPE type;
        RE::TESObjectBOOK* note;
        std::vector<RE::FormID> bossList;
        std::uint16_t objectiveIndex;
    };

    struct Reward
    {
        RE::FormID formID;
        std::string modName;
        std::unordered_map<Util::DIFFICULTY, std::uint32_t> amount;
    };

    static System* GetSingleton()
    {
        static System singleton;
        return &singleton;
    }

    void AddToQueue(std::shared_ptr<Quest> a_quest);
    auto CreateNote(std::string a_name, std::string a_difficulty) -> RE::TESObjectBOOK*;
    void CompleteObjective(RE::BGSLocation* a_region, std::uint16_t a_index);
    auto GetAliasReference(RE::TESQuest* a_quest, std::uint32_t a_index) -> RE::BGSBaseAlias*;
    auto GetMapMarker(RE::BGSLocation* a_location) -> RE::TESObjectREFR*;
    auto GetQuests() const -> const std::vector<std::shared_ptr<Quest>>&;
    void ParseQuests();
    void ParseRewards();
    void ParseTrackers();
    void ParseTexts();
    void PopulateMenu(RE::BGSLocation* a_region, Util::TYPE a_type);
    void RewardPlayer(RE::BGSLocation* a_region);
    void ShowGiftMenu(RE::TESObjectREFR* a_target, RE::TESObjectREFR* a_source);
    void StartEveryQuest(RE::BGSLocation* a_region, Util::TYPE a_type);
    static void StartQuests();
    void StartRandomQuest(RE::BGSLocation* a_region, Util::TYPE a_type);
    void UpdateGlobals();
    static void UpdateLocationAlias(RE::TESQuest* a_quest, RE::BGSLocation* a_location);
    void UpdateReward(RE::TESQuest* a_quest, std::uint16_t a_index);

    static void ForceLocationTo(RE::TESQuest* a_quest, std::uint32_t a_aliasID, RE::BGSLocation* a_location);
    static void ForceRefTo(RE::TESQuest* a_quest, std::uint32_t a_aliasID, RE::TESObjectREFR* a_reference);
    static bool GetIsEditorLocation(RE::BGSLocation* a_location, RE::Actor* a_actor, bool a_unk03 = false);
    static auto GetRefTypeAliveCount(RE::BGSLocation* a_location, RE::BGSLocationRefType* a_type, std::uint32_t a_unk03, std::uint32_t a_unk04, std::uint64_t a_unk05, std::uint32_t a_unk06, std::uint32_t a_unk07) -> std::uint32_t;
    static void SetObjectiveState(RE::BGSQuestObjective* a_objective, RE::QUEST_OBJECTIVE_STATE a_state);
private:
    System() = default;
    System(const System&) = delete;
    System(System&&) = delete;

    ~System() = default;

    System& operator=(const System&) = delete;
    System& operator=(System&&) = delete;

    std::vector<Reward> rewards;
    std::vector<std::shared_ptr<Quest>> quests;
    std::vector<std::shared_ptr<Quest>> queue;
};