#pragma once

using EventResult = RE::BSEventNotifyControl;

class Events final : 
    public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
    public RE::BSTEventSink<RE::TESActorLocationChangeEvent>, 
    public RE::BSTEventSink<RE::TESContainerChangedEvent>
    
{
public:
    static Events* GetSingleton();
    static void Register();
private:
    EventResult ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
    EventResult ProcessEvent(const RE::TESActorLocationChangeEvent* a_event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*) override;
    EventResult ProcessEvent(const RE::TESContainerChangedEvent* a_event, RE::BSTEventSource<RE::TESContainerChangedEvent>*) override;
    

    Events() = default;
    Events(const Events&) = delete;
    Events(Events&&) = delete;

    ~Events() override = default;

    Events& operator=(const Events&) = delete;
    Events& operator=(Events&&) = delete;
};