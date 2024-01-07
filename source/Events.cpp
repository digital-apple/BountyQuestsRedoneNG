#include "Events.h"
#include "Util.h"
#include "System.h"
#include "Serialization.h"

Events* Events::GetSingleton()
{
    static Events singleton;
    return std::addressof(singleton);
}

void Events::Register()
{

    if (const auto events = RE::ScriptEventSourceHolder::GetSingleton()) {
        events->GetEventSource<RE::TESActorLocationChangeEvent>()->AddEventSink(GetSingleton());
        events->GetEventSource<RE::TESContainerChangedEvent>()->AddEventSink(GetSingleton());

        auto UI = RE::UI::GetSingleton();
        if (UI) {
            UI->AddEventSink<RE::MenuOpenCloseEvent>(GetSingleton());
        }

        logs::info("Events::Register :: Registered for MenuOpenCloseEvent");
        logs::info("Events::Register :: Registered for TESActorLocationChangeEvent");
        logs::info("Events::Register :: Registered for TESContainerChangedEvent");
    }
}

EventResult Events::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
    if (!a_event) {
        return EventResult::kContinue;
    }

    if (a_event->menuName == RE::GiftMenu::MENU_NAME && !a_event->opening) {
        System::GetSingleton()->ParseQueue();
    }

    if (a_event->menuName == RE::DialogueMenu::MENU_NAME && a_event->opening) {
        System::GetSingleton()->UpdateGlobals();
    }

    return EventResult::kContinue;
}

EventResult Events::ProcessEvent(const RE::TESActorLocationChangeEvent* a_event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*)
{
    if (!a_event) {
        return EventResult::kContinue;
    }

    if (auto actor = a_event->actor.get(); actor) {
        if (actor == RE::PlayerCharacter::GetSingleton()) {

            if (auto newLocation = a_event->newLoc; newLocation) {
                logs::info("Events::TESActorLocationChangeEvent :: Passing location: '{}'", newLocation->GetName());
                auto trackers = Serialization::GetSingleton()->GetTrackers();

                for (auto& tracker : trackers) {
                    auto currentLocation = newLocation;
                    logs::info("Events::TESActorLocationChangeEvent :: Current Tracker: '{:x}' | '{}' | '{:x}'", tracker->global->GetFormID(), tracker->region->GetName(), tracker->region->GetFormID());
                    while (currentLocation) {
                        if (tracker->region == currentLocation) {
                            logs::info("Events::TESActorLocationChangeEvent :: Found parent region: '{}' | '{:x}'", currentLocation->GetName(), currentLocation->GetFormID());
                            const auto BQRNG_Catalogue = Util::GetSingleton()->GetQuest(Offsets::Forms::BQRNG_Catalogue, "Bounty Quests Redone - NG.esl");
                            System::GetSingleton()->UpdateLocationAlias(BQRNG_Catalogue, currentLocation);
                            break;
                        }
                        currentLocation = currentLocation->parentLoc;
                    }
                }
            }
        }
    }

    return EventResult::kContinue;
}

EventResult Events::ProcessEvent(const RE::TESContainerChangedEvent* a_event, RE::BSTEventSource<RE::TESContainerChangedEvent>*)
{
    if (!a_event) {
        return EventResult::kContinue;
    }

    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player || !player->Is3DLoaded()) {
        return EventResult::kContinue;
    }

    if (const auto baseObject = a_event->baseObj; baseObject && a_event->newContainer == player->GetFormID()) {
        auto quests = System::GetSingleton()->GetQuests();
        for (auto& quest : quests) {
            if (const auto note = quest->note; note) {
                if (note->GetFormID() == baseObject) {
                    System::GetSingleton()->AddToQueue(std::move(quest));
                    RE::PlayerCharacter::GetSingleton()->RemoveItem(note, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                }
            }
        }
    }

    return EventResult::kContinue;
}