#pragma once

namespace Offsets
{
    namespace Addresses
    {
        static REL::ID ShowGiftMenu_Target{ static_cast<std::uint64_t>(REL::Relocate(519570, 406111)) };
        static REL::ID ShowGiftMenu_Source{ static_cast<std::uint64_t>(REL::Relocate(519571, 406112)) };
        static constexpr auto Native_ForceLocationTo{ RELOCATION_ID(24525, 25054) };
        static constexpr auto Native_ForceRefTo{ RELOCATION_ID(24523, 25052) };
        static constexpr auto Native_GetIsEditorLocation{ RELOCATION_ID(17961, 18365) };
        static constexpr auto Native_GetRefTypeAliveCount{ RELOCATION_ID(17964, 18368) };
        static constexpr auto Native_SetObjectiveState{ RELOCATION_ID(23467, 23933) };
    }

    namespace Forms
    {
        static RE::FormID BQRNG_AliasGenerator{ 0x10004B };
        static RE::FormID BQRNG_Catalogue{ 0x110000 };

        static RE::FormID BQRNG_RegionHasBandit{ 0x111000 };
        static RE::FormID BQRNG_RegionHasDragon{ 0x111001 };
        static RE::FormID BQRNG_RegionHasDraugr{ 0x111002 };
        static RE::FormID BQRNG_RegionHasDwarven{ 0x111003 };
        static RE::FormID BQRNG_RegionHasFalmer{ 0x111004 };
        static RE::FormID BQRNG_RegionHasForsworn{ 0x111005 };
        static RE::FormID BQRNG_RegionHasGiant{ 0x111006 };
        static RE::FormID BQRNG_RegionHasMage{ 0x111007 };
        static RE::FormID BQRNG_RegionHasReaver{ 0x111008 };
        static RE::FormID BQRNG_RegionHasRiekling{ 0x111009 };
        static RE::FormID BQRNG_RegionHasVampire{ 0x111010 };
        
        static RE::FormID BQRNG_NPC{ 0x100027 };

        static RE::FormID NoteInventoryModel{ 0x1541C };
        static RE::FormID NotePickupSound{ 0xC7A55 };
        static RE::FormID Boss{ 0x130F7 };
        static RE::FormID MapMarker{ 0x10F63C };
    }
}