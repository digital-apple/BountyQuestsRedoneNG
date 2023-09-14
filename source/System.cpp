#include "System.h"
#include "Serialization.h"
#include "Offsets.h"

void System::AddToQueue(std::shared_ptr<Quest> a_quest)
{
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

auto System::GetAliasReference(RE::TESQuest* a_quest) -> RE::Actor*
{
	if (a_quest) {
		for (auto& dummyAlias : a_quest->aliases) {
			if (dummyAlias->aliasID == 1) {
				return static_cast<RE::BGSRefAlias*>(dummyAlias)->GetActorReference();
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

			json config = json::parse(path);
			const json& questArray = config["Quests"];

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
				} else {
					logs::warn("System::ParseQuests :: Failed to parse quest '{}'", name);
				}
			}
		}
    }
}

void System::ParseQueue()
{
	std::jthread thread(&System::StartQuests, this);
	thread.detach();
}

void System::ParseRewards()
{
	std::ifstream path("Data/SKSE/Plugins/Bounty Quests Redone - NG/Rewards.json");
	json config = json::parse(path);
	const json& rewardArray = config["Rewards"];

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

void System::ParseTrackers()
{
	std::ifstream path("Data/SKSE/Plugins/Bounty Quests Redone - NG/Trackers.json");
	json config = json::parse(path);
	const json& trackerArray = config["Trackers"];

	const auto dataHandler = RE::TESDataHandler::GetSingleton();

	for (const auto& tracker : trackerArray.array_range()) {
		auto global = dataHandler->LookupForm<RE::TESGlobal>(tracker["GlobalVariable"]["FormID"].as<RE::FormID>(), tracker["GlobalVariable"]["ModName"].as<std::string>());
		auto region = dataHandler->LookupForm<RE::BGSLocation>(tracker["Region"]["FormID"].as<RE::FormID>(), tracker["Region"]["ModName"].as<std::string>());

		if (global && region) {
			Serialization::GetSingleton()->AddTracker(global, region);
		} else {
			logs::warn("System::ParseTrackers :: Failed to parse tracker: '{:x}'.", tracker["GlobalVariable"]["FormID"].as<RE::FormID>());
		}
	}
}

void System::PopulateMenu(RE::BGSLocation* a_region, Util::TYPE a_type)
{
	const auto player = RE::PlayerCharacter::GetSingleton();
	const auto npc = RE::TESDataHandler::GetSingleton()->LookupForm<RE::Actor>(Offsets::Forms::BQRNG_NPC, "Bounty Quests Redone - NG.esp");
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

void System::RewardPlayer(RE::BGSLocation* a_region)
{
	if (a_region) {
		auto data = Serialization::GetSingleton();
		for (auto& reward : rewards) {
			auto form = RE::TESDataHandler::GetSingleton()->LookupForm(reward.formID, reward.modName);

			if (form) {
				for (auto& amount : reward.amount) {
					auto times = data->GetTracker(a_region, amount.first);

					if (amount.second && times > 0U) {
						const auto quantity = amount.second * times;
						const auto sAddItemtoInventory = RE::GameSettingCollection::GetSingleton()->GetSetting("sAddItemtoInventory");

						RE::PlayerCharacter::GetSingleton()->AddObjectToContainer(form->As<RE::TESBoundObject>(), nullptr, quantity, nullptr);

						CompleteObjective(a_region, 0);
						
						if (sAddItemtoInventory) {
							std::string result = fmt::format("{} {}, {}", sAddItemtoInventory->GetString(), form->GetName(), quantity);
							RE::PlaySound("ITMGoldUpSD");
							RE::DebugNotification(result.c_str(), nullptr, true);
						}
					}
				}
			} else {
				logs::warn("System::RewardPlayer :: Invalid form: '{:x}' | '{}'", reward.formID, reward.modName);
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
	for (const auto& quest : quests) {
		if (quest->region == a_region && !Serialization::GetSingleton()->IsLocationReserved(quest->location)) {
			if (quest->type == a_type || a_type == Util::TYPE::None) {
				AddToQueue(quest);
			}
		}
	}
	ParseQueue();
}

void System::StartQuests()
{
	const auto BQRNG_AliasGenerator = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(Offsets::Forms::BQRNG_AliasGenerator, "Bounty Quests Redone - NG.esp");

	for (auto& quest : queue) {
		if (quest->quest && !quest->quest->IsRunning()) {
			quest->quest->Start();
		}

		for (const auto& alias : quest->quest->aliases) {
			const auto referenceAlias = reinterpret_cast<RE::BGSRefAlias*>(alias);
			if (!referenceAlias->GetActorReference()) {

				RE::Actor* reference = nullptr;
				auto counter = 0;
				
				while (!reference && counter < 10) {
					UpdateLocationAlias(BQRNG_AliasGenerator, quest->location);	
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					reference = GetAliasReference(BQRNG_AliasGenerator);
					counter++;
				}

				if (reference && !reference->IsDisabled() && !reference->IsDead()) {
					Serialization::GetSingleton()->ReserveLocation(quest->location, true);
					ForceRefTo(quest->quest, referenceAlias->aliasID, reference);

					RE::TESObjectREFR* worldMarker = quest->location->worldLocMarker.get().get();
					RE::TESObjectREFR* markerRef = worldMarker ? worldMarker : GetMapMarker(quest->location);
					if (markerRef) {
						RE::ExtraMapMarker* mapMarker = markerRef ? markerRef->extraList.GetByType<RE::ExtraMapMarker>() : nullptr;
						mapMarker->mapData->SetVisible(true);
					}
					
					for (const auto& objective : quest->quest->objectives) {
						if (objective->index == referenceAlias->aliasID) {
							objective->displayText = static_cast<std::string>(objective->displayText) + quest->location->GetName();
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
	}
	queue.clear();
}

void System::StartRandomQuest(RE::BGSLocation* a_region, Util::TYPE a_type)
{
	std::vector<std::shared_ptr<Quest>> temporary;

	for (const auto& quest : quests) {
		if (quest->region == a_region && !Serialization::GetSingleton()->IsLocationReserved(quest->location)) {
			if (quest->type == a_type || a_type == Util::TYPE::None) {
				temporary.push_back(quest);
			}
		}
	}

	const auto random = std::rand() % temporary.size();

	AddToQueue(quests[random]);
	ParseQueue();
}

void System::UpdateGlobals()
{
	const auto player = RE::PlayerCharacter::GetSingleton();
	const auto dataHandler = RE::TESDataHandler::GetSingleton();

	auto hasBandit = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasBandit, "Bounty Quests Redone - NG.esp");
	auto hasDragon = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasDragon, "Bounty Quests Redone - NG.esp");
	auto hasDraugr = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasDraugr, "Bounty Quests Redone - NG.esp");
	auto hasDwarven = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasDwarven, "Bounty Quests Redone - NG.esp");
	auto hasFalmer = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasFalmer, "Bounty Quests Redone - NG.esp");
	auto hasForsworn = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasForsworn, "Bounty Quests Redone - NG.esp");
	auto hasGiant = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasGiant, "Bounty Quests Redone - NG.esp");
	auto hasMage = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasMage, "Bounty Quests Redone - NG.esp");
	auto hasReaver = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasReaver, "Bounty Quests Redone - NG.esp");
	auto hasRiekling = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasRiekling, "Bounty Quests Redone - NG.esp");
	auto hasVampire = dataHandler->LookupForm<RE::TESGlobal>(Offsets::Forms::BQRNG_RegionHasVampire, "Bounty Quests Redone - NG.esp");

	hasBandit->value = 0U;
	hasDragon->value = 0U;
	hasDraugr->value = 0U;
	hasDwarven->value = 0U;
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
			} else if (quest->type == Util::TYPE::Dwarven) {
				hasDwarven->value = 1U;
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
				a_quest->Stop();
				auto location = static_cast<RE::BGSLocAlias*>(alias);
				location->unk28 = reinterpret_cast<uint64_t>(a_location);
				bool result;
				a_quest->EnsureQuestStarted(result, false);
				break;
			}
		}
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