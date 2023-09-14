#include "Util.h"

auto Util::GetCatalogue() const -> RE::TESQuest*
{
	return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(Offsets::Forms::BQRNG_Catalogue, "Bounty Quests Redone - NG.esp");
}

auto Util::GetDifficulty(std::string a_string) -> Util::DIFFICULTY
{
	std::transform(std::begin(a_string), std::end(a_string), std::begin(a_string), [](unsigned char c)->unsigned char { return static_cast<unsigned char>(std::tolower(c)); });
	if (a_string == "novice") {
		return DIFFICULTY::Novice;
	} else if (a_string == "apprentice") {
		return DIFFICULTY::Apprentice;
	} else if (a_string == "adept") {
		return DIFFICULTY::Adept;
	} else if (a_string == "expert") {
		return DIFFICULTY::Expert;
	} else if (a_string == "master") {
		return DIFFICULTY::Master;
	} else if (a_string == "legendary") {
		return DIFFICULTY::Legendary;
	} else {
		return DIFFICULTY::None;
	}
}

auto Util::GetLocation(RE::FormID a_formID, std::string a_modName) const -> RE::BGSLocation*
{
	return RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSLocation>(a_formID, a_modName);
}

auto Util::GetQuest(RE::FormID a_formID, std::string a_modName) const -> RE::TESQuest*
{
	return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(a_formID, a_modName);
}

auto Util::GetType(std::string a_string) -> Util::TYPE
{
	std::transform(std::begin(a_string), std::end(a_string), std::begin(a_string), [](unsigned char c)->unsigned char { return static_cast<unsigned char>(std::tolower(c)); });
	if (a_string == "bandit") {
		return TYPE::Bandit;
	} else if (a_string == "dragon") {
		return TYPE::Dragon;
	} else if (a_string == "draugr") {
		return TYPE::Draugr;
	} else if (a_string == "dwarven") {
		return TYPE::Dwarven;
	} else if (a_string == "falmer") {
		return TYPE::Falmer;
	} else if (a_string == "forsworn") {
		return TYPE::Forsworn;
	} else if (a_string == "giant") {
		return TYPE::Giant;
	} else if (a_string == "mage") {
		return TYPE::Mage;
	} else if (a_string == "reaver") {
		return TYPE::Reaver;
	} else if (a_string == "riekling") {
		return TYPE::Riekling;
	} else if (a_string == "vampire") {
		return TYPE::Vampire;
	} else {
		return TYPE::None;
	}
}
