#include "Papyrus.h"
#include "System.h"
#include "Util.h"

constexpr std::string_view PapyrusClass = "BQRNG";

void RewardPlayer(RE::StaticFunctionTag*, RE::BGSLocation* a_region)
{
    System::GetSingleton()->RewardPlayer(a_region);
}

void ShowMenu(RE::StaticFunctionTag*, RE::BGSLocation* a_region, std::uint32_t a_type)
{
    if (a_region) {
        const auto type = static_cast<Util::TYPE>(a_type);
        System::GetSingleton()->PopulateMenu(a_region, type);    
    }
}

void StartEveryQuest(RE::StaticFunctionTag*, RE::BGSLocation* a_region, std::uint32_t a_type = 0)
{
    if (a_region) {
        const auto type = static_cast<Util::TYPE>(a_type);
        System::GetSingleton()->StartEveryQuest(a_region, type);
    }
}

void StartRandomQuest(RE::StaticFunctionTag*, RE::BGSLocation* a_region, std::uint32_t a_type)
{
    if (a_region) {
        const auto type = static_cast<Util::TYPE>(a_type);
        System::GetSingleton()->StartRandomQuest(a_region, type);
    }
}

void UpdateReward(RE::StaticFunctionTag*, RE::TESQuest* a_quest, std::uint16_t a_index)
{
    if (a_quest) {
        System::GetSingleton()->UpdateReward(a_quest, a_index);
    }
}

bool Papyrus::RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm)
{
    a_vm->RegisterFunction("RewardPlayer", PapyrusClass, RewardPlayer);
    a_vm->RegisterFunction("ShowMenu", PapyrusClass, ShowMenu);
    a_vm->RegisterFunction("StartEveryQuest", PapyrusClass, StartEveryQuest);
    a_vm->RegisterFunction("StartRandomQuest", PapyrusClass, StartRandomQuest);
    a_vm->RegisterFunction("UpdateReward", PapyrusClass, UpdateReward);
    
    return true;
}