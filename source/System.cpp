#include "System.h"
#include "RE/B/BGSLocAlias.h"
#include "Serialization.h"
#include "Offsets.h"

#undef PlaySound

void System::AddToQueue(std::shared_ptr<Quest> a_quest)
{
    INFO("System::AddToQueue :: Adding quest: '{}' to the queue.", a_quest->location->GetName());
    queue.push_back(a_quest);
}

auto System::CreateNote(std::string a_name, std::string a_difficulty) -> RE::TESObjectBOOK*
{
    const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESObjectBOOK>();
    auto note = factory->Create();
    note->fullName = a_difficulty + " - " + a_name;
    note->inventoryModel = RE::TESForm::LookupByID<RE::TESObjectSTAT>(Offsets::Forms::NoteInventoryModel);
    note->pickupSound = RE::TESForm::LookupByID<RE::BGSSoundDescriptorForm>(Offsets::Forms::NotePickupSound);

    return note;
}

void System::CompleteObjective(RE::BGSLocation* a_region, std::uint16_t a_index)
{
    for (auto& quest : quests) {
        if (a_region == quest->region) {
            for (auto& objective : quest->quest->objectives) {
                if (objective->index == a_index) {
                    SetObjectiveState(objective, RE::QUEST_OBJECTIVE_STATE::kCompletedDisplayed );
                    SetObjectiveState(objective, RE::QUEST_OBJECTIVE_STATE::kDormant );
                }
            }        
        }
    }
}

auto System::GetAliasReference(RE::TESQuest* a_quest, std::uint32_t a_index) -> RE::BGSBaseAlias*
{
    if (a_quest) {
        for (const auto& alias : a_quest->aliases) {
            if (alias && alias->aliasID == a_index) {
                return static_cast<RE::BGSRefAlias*>(alias);
            }
        }
    }
    return nullptr;
}

auto System::GetMapMarker(RE::BGSLocation* a_location) -> RE::TESObjectREFR*
{
    const auto keyword = RE::TESForm::LookupByID<RE::BGSLocationRefType>(Offsets::Forms::MapMarker);
    for (const auto& reference : a_location->specialRefs) {
        if (reference.type == keyword) {
            return RE::TESForm::LookupByID<RE::TESObjectREFR>(reference.refData.refID);
        }
    }
    return nullptr;
}

auto System::GetQuests() const -> const std::vector<std::shared_ptr<Quest>>&
{
    return quests;
}

void System::ParseQuests()
{
    const std::filesystem::path source{"Data/SKSE/Plugins/Bounty Quests Redone - NG/Quests"};

    for (auto const& entry : std::filesystem::directory_iterator{source}) 
    {
        if (entry.path().extension() == ".json") {
            std::ifstream path(std::filesystem::absolute(entry.path()));

            jsoncons::json config = jsoncons::json::parse(path);
            const jsoncons::json& questArray = config["Quests"];

            const auto util = Util::GetSingleton();

            for (const auto& quest : questArray.array_range()) {
                auto name = quest["LocationName"].as<std::string>();
                auto difficulty = util->GetDifficulty(quest["Difficulty"].as<std::string>());
                auto location = util->GetLocation(quest["Location"]["FormID"].as<RE::FormID>(), quest["Location"]["ModName"].as<std::string>());
                auto region = util->GetLocation(quest["Region"]["FormID"].as<RE::FormID>(), quest["Region"]["ModName"].as<std::string>());
                auto owner = util->GetQuest(quest["Quest"]["FormID"].as<RE::FormID>(), quest["Quest"]["ModName"].as<std::string>());
                auto type = util->GetType(quest["Type"].as<std::string>());

                if (location && region && owner) {
                    auto note = CreateNote(name, quest["Difficulty"].as<std::string>());
                    quests.push_back(std::make_shared<Quest>(name, difficulty, location, region, owner, type, note));
                    INFO("System::ParseQuests :: Successfully parsed quest: '{}' with type: '{}' and difficulty: '{}' from: '{}'", name, quest["Type"].as<std::string>(), quest["Difficulty"].as<std::string>(), quest["Quest"]["ModName"].as<std::string>());
                } else {
                    WARN("System::ParseQuests :: Failed to parse quest: '{}'", name);
                }
            }
        }
    }
}

void System::ParseRewards()
{
    std::ifstream path("Data/SKSE/Plugins/Bounty Quests Redone - NG/Rewards.json");
    jsoncons::json config = jsoncons::json::parse(path);
    const jsoncons::json& rewardArray = config["Rewards"];

    for (const auto& reward : rewardArray.array_range()) {

        auto formID = reward["FormID"].as<RE::FormID>();
        auto modName = reward["ModName"].as<std::string>();

        std::unordered_map<Util::DIFFICULTY, std::uint32_t> amount;

        amount[Util::DIFFICULTY::Novice] = reward["Quantity"]["Novice"].as<std::uint32_t>();
        amount[Util::DIFFICULTY::Apprentice] = reward["Quantity"]["Apprentice"].as<std::uint32_t>();
        amount[Util::DIFFICULTY::Adept] = reward["Quantity"]["Adept"].as<std::uint32_t>();
        amount[Util::DIFFICULTY::Expert] = reward["Quantity"]["Expert"].as<std::uint32_t>();
        amount[Util::DIFFICULTY::Master] = reward["Quantity"]["Master"].as<std::uint32_t>();
        amount[Util::DIFFICULTY::Legendary] = reward["Quantity"]["Legendary"].as<std::uint32_t>();

        Reward instance{ formID, modName, amount };
        rewards.push_back(instance);
    }
}

void System::ParseTexts()
{
    const auto util = Util::GetSingleton();

    std::ifstream path("Data/SKSE/Plugins/Bounty Quests Redone - NG/Texts.json");
    jsoncons::json config = jsoncons::json::parse(path);
    const jsoncons::json& textArray = config["Texts"];

    for (const auto& text : textArray.array_range()) {
        util->SetText(Util::TEXT::Objective, text["Objective"].as<std::string>());
        util->SetText(Util::TEXT::Novice, text["Novice"].as<std::string>());
        util->SetText(Util::TEXT::Apprentice, text["Apprentice"].as<std::string>());
        util->SetText(Util::TEXT::Adept, text["Adept"].as<std::string>());
        util->SetText(Util::TEXT::Expert, text["Expert"].as<std::string>());
        util->SetText(Util::TEXT::Master, text["Master"].as<std::string>());
        util->SetText(Util::TEXT::Legendary, text["Legendary"].as<std::string>());
    }
}

void System::ParseTrackers()
{
    std::ifstream path("Data/SKSE/Plugins/Bounty Quests Redone - NG/Trackers.json");
    jsoncons::json config = jsoncons::json::parse(path);
    const jsoncons::json& trackerArray = config["Trackers"];

    const auto dataHandler = RE::TESDataHandler::GetSingleton();

    for (const auto& tracker : trackerArray.array_range()) {
        auto global = dataHandler->LookupForm<RE::TESGlobal>(tracker["GlobalVariable"]["FormID"].as<RE::FormID>(), tracker["GlobalVariable"]["ModName"].as<std::string>());
        auto region = dataHandler->LookupForm<RE::BGSLocation>(tracker["Region"]["FormID"].as<RE::FormID>(), tracker["Region"]["ModName"].as<std::string>());

        if (global && region) {
            Serialization::GetSingleton()->AddTracker(global, region);
        } else {
            WARN("System::ParseTrackers :: Failed to parse tracker: '0x{:x}'.", tracker["GlobalVariable"]["FormID"].as<RE::FormID>());
        }
    }
}

void System::PopulateMenu(RE::BGSLocation* a_region, Util::TYPE a_type)
{
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto npc = RE::TESDataHandler::GetSingleton()->LookupForm<RE::Actor>(Offsets::Forms::BQRNG_NPC, "Bounty Quests Redone - NG.esl");

    if (npc) {
        npc->ResetInventory(false);

        for (auto& quest : quests) {
            if (quest->region == a_region && quest->type == a_type) {
                const auto keyword = RE::TESForm::LookupByID<RE::BGSLocationRefType>(Offsets::Forms::Boss);

                if (GetRefTypeAliveCount(quest->location, keyword, 0, 0, 0, 1, 0) > 0 && !Serialization::GetSingleton()->IsLocationReserved(quest->location) && GetIsEditorLocation(quest->region, player)) {
                    npc->AddObjectToContainer(quest->note, nullptr, 1, nullptr);
                }
            }
        }
        ShowGiftMenu(npc, player);
    }
}

void System::RewardPlayer(RE::BGSLocation* a_region)
{
    if (a_region) {
        auto data = Serialization::GetSingleton();
        for (auto& reward : rewards) {
            auto form = RE::TESDataHandler::GetSingleton()->LookupForm(reward.formID, reward.modName);

            if (form) {
                for (auto& amount : reward.amount) {
                    std::uint32_t times = 0U;
                    auto trackers = data->GetTrackers();

                    for (auto& tracker : trackers) {
                        if (tracker->region == a_region) {
                            times = tracker->reward[amount.first];
                        }
                    }

                    if (amount.second && times > 0U) {
                        const auto quantity = amount.second * times;
                        const auto sAddItemtoInventory = RE::GameSettingCollection::GetSingleton()->GetSetting("sAddItemtoInventory");

                        RE::PlayerCharacter::GetSingleton()->AddObjectToContainer(form->As<RE::TESBoundObject>(), nullptr, quantity, nullptr);

                        CompleteObjective(a_region, 0);
                        
                        if (sAddItemtoInventory) {
                            std::string result = std::format("{} {}, {}", sAddItemtoInventory->GetString(), form->GetName(), quantity);
                            RE::PlaySound("ITMGoldUpSD");
                            RE::DebugNotification(result.c_str(), nullptr, true);
                        }
                    }
                }
            } else {
                WARN("System::RewardPlayer :: Invalid form: '0x{:x}' | '{}'", reward.formID, reward.modName);
            }
        }
        data->ClearTracker(a_region);
    }
}

void System::ShowGiftMenu(RE::TESObjectREFR* a_target, RE::TESObjectREFR* a_source)
{
    const auto giftMenu = RE::UI::GetSingleton()->GetMenu<RE::GiftMenu>();

    auto target = reinterpret_cast<RE::ObjectRefHandle*>(Offsets::Addresses::ShowGiftMenu_Target.address());
    auto source = reinterpret_cast<RE::ObjectRefHandle*>(Offsets::Addresses::ShowGiftMenu_Source.address());

    *target = a_target->GetHandle();
    *source = a_source->GetHandle();

    const auto messageQueue = RE::UIMessageQueue::GetSingleton();
    const auto strings = RE::InterfaceStrings::GetSingleton();
    messageQueue->AddMessage(strings->giftMenu, RE::UI_MESSAGE_TYPE::kShow, nullptr);
}

void System::StartEveryQuest(RE::BGSLocation* a_region, Util::TYPE a_type)
{
    INFO("System::StartEveryQuest :: Starting every available quest from: '{}' with type: '{}'", a_region->GetName(), static_cast<std::uint32_t>(a_type));

    for (const auto& quest : quests) {
        if (quest->region == a_region && !Serialization::GetSingleton()->IsLocationReserved(quest->location)) {
            if (quest->type == a_type || a_type == Util::TYPE::None) {
                AddToQueue(quest);
            }
        }
    }

    std::jthread thread(&System::StartQuests);
    thread.detach();
}

void System::StartQuests()
{
    const auto BQRNG_AliasGenerator = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(Offsets::Forms::BQRNG_AliasGenerator, "Bounty Quests Redone - NG.esl");

    auto system = GetSingleton();

    INFO("System::StartQuests :: Parsing '{}' quests.", system->queue.size());

    for (auto& quest : system->queue) {
        if (quest->location && quest->region && quest->quest) {
            if (quest->quest && !quest->quest->IsRunning()) {
                quest->quest->Start();
            }

            for (const auto& alias : quest->quest->aliases) {
                const auto referenceAlias = reinterpret_cast<RE::BGSRefAlias*>(alias);

                if (!referenceAlias->GetActorReference() || (referenceAlias->GetActorReference() && referenceAlias->GetActorReference()->IsDead())) {
                    RE::Actor* reference = nullptr;
                    std::size_t counter = 0;
                
                    while (!reference && counter < 5) {

                        system->UpdateLocationAlias(BQRNG_AliasGenerator, quest->location);

                        std::this_thread::sleep_for(std::chrono::milliseconds(250));

                        reference = static_cast<RE::BGSRefAlias*>(system->GetAliasReference(BQRNG_AliasGenerator, 1U))->GetActorReference();
                        counter++;
                        
                    }

                    if (reference && !reference->IsDisabled() && !reference->IsDead()) {
                        Serialization::GetSingleton()->ReserveLocation(quest->location, true);
                        ForceRefTo(quest->quest, referenceAlias->aliasID, reference);
                        RE::TESObjectREFR* worldMarker = quest->location->worldLocMarker.get().get();
                        RE::TESObjectREFR* markerRef = worldMarker ? worldMarker : system->GetMapMarker(quest->location);

                        if (markerRef) {
                            RE::ExtraMapMarker* mapMarker = markerRef ? markerRef->extraList.GetByType<RE::ExtraMapMarker>() : nullptr;
                            mapMarker->mapData->SetVisible(true);
                        }
                    
                        for (const auto& objective : quest->quest->objectives) {
                            if (objective->index == referenceAlias->aliasID) {
                                auto util = Util::GetSingleton();
                                auto text = util->GetText(Util::TEXT::Objective);

                                auto result = text.replace(text.find("%d"), 2, util->GetDifficulty(quest->difficulty));

                                if (objective->index < 10) {
                                    result = result.replace(result.find("%i"), 2, "0" + std::to_string(objective->index));
                                } else {
                                    result = result.replace(result.find("%i"), 2, std::to_string(objective->index));
                                }

                                result = result.replace(result.find("%l"), 2, quest->location->GetName());

                                objective->displayText = result;

                                Serialization::GetSingleton()->SerializeObjectivesText(quest->quest, quest->location, objective->index, objective->displayText.c_str());
                                quest->objectiveIndex = objective->index;
                                SetObjectiveState(objective, RE::QUEST_OBJECTIVE_STATE::kDisplayed);
                            }
                        }
                        break;
                    }
                    break;
                }
            }
        } else {
            WARN("System::StartQuests :: Quest: '{}' couldn't be started due to missing or invalid data.", quest->name);
        }
        system->queue.clear();
    }
}

void System::StartRandomQuest(RE::BGSLocation* a_region, Util::TYPE a_type)
{
    INFO("System::StartRandomQuest :: Starting random quest from: '{}' with type: '{}'", a_region->GetName(), static_cast<std::uint32_t>(a_type));

    std::vector<std::shared_ptr<Quest>> temporary;

    for (const auto& quest : quests) {
        if (quest->region == a_region && !Serialization::GetSingleton()->IsLocationReserved(quest->location)) {
            if (quest->type == a_type || a_type == Util::TYPE::None) {
                temporary.push_back(quest);
            }
        }
    }

    const auto random = std::rand() % temporary.size();

    AddToQueue(temporary[random]);

    std::jthread thread(&System::StartQuests);
    thread.detach();
}

void System::UpdateGlobals()
{
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto dataHandler = RE::TESDataHandler::GetSingleton();

    auto hasBandit = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasBandit, "Bounty Quests Redone - NG.esl");
    auto hasDragon = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasDragon, "Bounty Quests Redone - NG.esl");
    auto hasDraugr = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasDraugr, "Bounty Quests Redone - NG.esl");
    auto hasDwemer = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasDwemer, "Bounty Quests Redone - NG.esl");
    auto hasFalmer = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasFalmer, "Bounty Quests Redone - NG.esl");
    auto hasForsworn = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasForsworn, "Bounty Quests Redone - NG.esl");
    auto hasGiant = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasGiant, "Bounty Quests Redone - NG.esl");
    auto hasMage = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasMage, "Bounty Quests Redone - NG.esl");
    auto hasReaver = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasReaver, "Bounty Quests Redone - NG.esl");
    auto hasRiekling = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasRiekling, "Bounty Quests Redone - NG.esl");
    auto hasVampire = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasVampire, "Bounty Quests Redone - NG.esl");

    hasBandit->value = 0U;
    hasDragon->value = 0U;
    hasDraugr->value = 0U;
    hasDwemer->value = 0U;
    hasFalmer->value = 0U;
    hasForsworn->value = 0U;
    hasGiant->value = 0U;
    hasMage->value = 0U;
    hasReaver->value = 0U;
    hasRiekling->value = 0U;
    hasVampire->value = 0U;
    
    for (const auto& quest : quests) {
        if (GetIsEditorLocation(quest->region, player)) {
            if (quest->type == Util::TYPE::Bandit) {
                hasBandit->value = 1U;
            } else if (quest->type == Util::TYPE::Dragon) {
                hasDragon->value = 1U;
            } else if (quest->type == Util::TYPE::Draugr) {
                hasDraugr->value = 1U;
            } else if (quest->type == Util::TYPE::Dwemer) {
                hasDwemer->value = 1U;
            } else if (quest->type == Util::TYPE::Falmer) {
                hasFalmer->value = 1U;
            } else if (quest->type == Util::TYPE::Forsworn) {
                hasForsworn->value = 1U;
            } else if (quest->type == Util::TYPE::Giant) {
                hasGiant->value = 1U;
            } else if (quest->type == Util::TYPE::Mage) {
                hasMage->value = 1U;
            } else if (quest->type == Util::TYPE::Reaver) {
                hasReaver->value = 1U;
            } else if (quest->type == Util::TYPE::Riekling) {
                hasRiekling->value = 1U;
            } else if (quest->type == Util::TYPE::Vampire) {
                hasVampire->value = 1U;
            }
        }
    }
}

void System::UpdateLocationAlias(RE::TESQuest* a_quest, RE::BGSLocation* a_location)
{
    if (a_quest && a_location) {
        for (auto& alias : a_quest->aliases) {
            if (alias->aliasID == 0) {
                auto locationAlias = static_cast<RE::BGSLocAlias*>(alias);

                a_quest->Stop();

                INFO("System::UpdateLocationAlias :: Stopped quest: '{}' | '0x{:x}'", a_quest->GetName(), a_quest->GetFormID());
                
                if (locationAlias) {
                    std::size_t counter = 5;
                    while ((a_quest->IsStopped() || !locationAlias->unk28) && counter > 0) {
                        INFO("System::UpdateLocationAlias :: Attempting to set alias: '{}' on: '{}' | '0x{:x} with location: '{}' | '0x{:x}' Tries left: '{}'", alias->aliasID, a_quest->GetName(), a_quest->GetFormID(), a_location->GetName(), a_location->GetFormID(), counter);
                        a_quest->Stop();
                        locationAlias->unk28 = reinterpret_cast<std::uint64_t>(a_location);
                        bool result;
                        a_quest->EnsureQuestStarted(result, false);
                        counter--;
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    }
                }
            }
        }
    } else {
        INFO("System::UpdateLocationAlias :: Invalid quest or location!");
    }
}
void System::UpdateReward(RE::TESQuest* a_quest, std::uint16_t a_index)
{
    for (auto& quest : quests) {
        if (quest->quest == a_quest && quest->objectiveIndex == a_index) {

            CompleteObjective(quest->region, quest->objectiveIndex);

            Serialization::GetSingleton()->ReserveLocation(quest->location, false);
            Serialization::GetSingleton()->SetTracker(quest->region, quest->difficulty, 1U);

            for (auto& objective : quest->quest->objectives) {
                if (objective->index == 0 && !objective->state.any(RE::QUEST_OBJECTIVE_STATE::kDisplayed)) {
                    SetObjectiveState(objective, RE::QUEST_OBJECTIVE_STATE::kDisplayed);
                }
            }

            break;
        }
    }
}

void System::ForceLocationTo(RE::TESQuest* a_quest, std::uint32_t a_aliasID, RE::BGSLocation* a_location)
{
    using func_t = decltype(&ForceLocationTo);
    REL::Relocation<func_t> func{ Offsets::Addresses::Native_ForceLocationTo };
    return func(a_quest, a_aliasID, a_location);
}

void System::ForceRefTo(RE::TESQuest* a_quest, std::uint32_t a_aliasID, RE::TESObjectREFR* a_reference)
{
    using func_t = decltype(&ForceRefTo);
    REL::Relocation<func_t> func{ Offsets::Addresses::Native_ForceRefTo };
    return func(a_quest, a_aliasID, a_reference);
}

bool System::GetIsEditorLocation(RE::BGSLocation* a_location, RE::Actor* a_actor, bool a_unk03)
{
    using func_t = decltype(&GetIsEditorLocation);
    REL::Relocation<func_t> func{ Offsets::Addresses::Native_GetIsEditorLocation };
    return func(a_location, a_actor, a_unk03);
}

auto System::GetRefTypeAliveCount(RE::BGSLocation* a_location, RE::BGSLocationRefType* a_type, std::uint32_t a_unk03, std::uint32_t a_unk04, std::uint64_t a_unk05, std::uint32_t a_unk06, std::uint32_t a_unk07) -> std::uint32_t
{
    using func_t = decltype(&GetRefTypeAliveCount);
    REL::Relocation<func_t> func{ Offsets::Addresses::Native_GetRefTypeAliveCount };
    return func(a_location, a_type, a_unk03, a_unk04, a_unk05, a_unk06, a_unk07);
}

void System::SetObjectiveState(RE::BGSQuestObjective* a_objective, RE::QUEST_OBJECTIVE_STATE a_state)
{
    using func_t = decltype(&SetObjectiveState);
    REL::Relocation<func_t> func{ Offsets::Addresses::Native_SetObjectiveState };
    return func(a_objective, a_state);
}