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

        INFO("Events::Register :: Registered for MenuOpenCloseEvent");
        INFO("Events::Register :: Registered for TESActorLocationChangeEvent");
        INFO("Events::Register :: Registered for TESContainerChangedEvent");
    }
}

EventResult Events::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
    if (!a_event) {
        return EventResult::kContinue;
    }

    if (a_event->menuName == RE::GiftMenu::MENU_NAME && !a_event->opening) {
        std::jthread thread(&System::StartQuests);
        thread.detach();
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
                INFO("Events::TESActorLocationChangeEvent :: Passing location: '{}' | '0x{:x}'", newLocation->GetName(), newLocation->GetFormID());
                auto trackers = Serialization::GetSingleton()->GetTrackers();

                for (auto& tracker : trackers) {
                    auto currentLocation = newLocation;
                    while (currentLocation) {
                        if (tracker->region == currentLocation) {
                            INFO("Events::TESActorLocationChangeEvent :: Found parent region: '{}' | '0x{:x}'", currentLocation->GetName(), currentLocation->GetFormID());
                            const auto BQRNG_Catalogue = Util::GetSingleton()->GetQuest(Offsets::Forms::BQRNG_Catalogue, "Bounty Quests Redone - NG.esl");

                            std::jthread thread(&System::UpdateLocationAlias, BQRNG_Catalogue, currentLocation);
                            thread.detach();
                            
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