#pragma once

#include "Offsets.h"

class Util
{
public:
	enum class TYPE : std::uint32_t
	{
		None = 0,
		Bandit = 1,
		Dragon = 2,
		Draugr = 3,
		Dwarven = 4,
		Falmer = 5,
		Forsworn = 6,
		Giant = 7,
		Mage = 8,
		Reaver = 9,
		Riekling = 10,
		Vampire = 11
	};

	enum class DIFFICULTY : std::uint32_t
	{
		None = 0,
		Novice = 1,
		Apprentice = 2,
		Adept = 3,
		Expert = 4,
		Master = 5,
		Legendary = 6
	};

	static Util* GetSingleton()
	{
		static Util singleton;
		return &singleton;
	}

	auto GetCatalogue() const -> RE::TESQuest*;
	auto GetDifficulty(std::string a_string) -> DIFFICULTY;
	auto GetLocation(RE::FormID a_formID, std::string a_modName) const -> RE::BGSLocation*;
	auto GetQuest(RE::FormID a_formID, std::string a_modName) const -> RE::TESQuest*;
	auto GetType(std::string a_string) -> TYPE;
private:
	Util() = default;
	Util(const Util&) = delete;
	Util(Util&&) = delete;

	~Util() = default;

	Util& operator=(const Util&) = delete;
	Util& operator=(Util&&) = delete;
};