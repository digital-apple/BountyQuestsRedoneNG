#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <jsoncons/json.hpp>

#define ERROR(message, ...) SKSE::log::error(message, ##__VA_ARGS__)
#define INFO(message, ...) SKSE::log::info(message, ##__VA_ARGS__)
#define TRACE(message, ...) SKSE::log::trace(message, ##__VA_ARGS__)
#define WARN(message, ...) SKSE::log::warn(message, ##__VA_ARGS__)

namespace stl
{
    using namespace SKSE::stl;

    template <class T, std::size_t index, class U>
    void write_vfunc()
    {
        REL::Relocation VTABLE{ T::VTABLE[0] };
        U::Callback = VTABLE.write_vfunc(index, U::Call);
    }

    template <class T>
    void write_vfunc_call(std::uintptr_t a_source)
    {
        auto& trampoline = SKSE::GetTrampoline();
        T::Callback = *reinterpret_cast<std::uintptr_t*>(trampoline.write_call<6>(a_source, T::Call));
    }

    template <class T>
    void write_thunk_call(std::uintptr_t a_source)
    {
        auto& trampoline = SKSE::GetTrampoline();
        T::Callback = trampoline.write_call<5>(a_source, T::Call);
    }
}