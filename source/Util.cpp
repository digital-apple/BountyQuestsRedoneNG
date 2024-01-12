#include "Util.h"

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

auto Util::GetDifficulty(Util::DIFFICULTY a_difficulty) -> std::string
{
    switch (a_difficulty) {
    case DIFFICULTY::Novice:
        return GetText(Util::TEXT::Novice);
        break;
    case DIFFICULTY::Apprentice:
        return GetText(Util::TEXT::Apprentice);
        break;
    case DIFFICULTY::Adept:
        return GetText(Util::TEXT::Adept);
        break;
    case DIFFICULTY::Expert:
        return GetText(Util::TEXT::Expert);
        break;
    case DIFFICULTY::Master:
        return GetText(Util::TEXT::Master);
        break;
    case DIFFICULTY::Legendary:
        return GetText(Util::TEXT::Legendary);
        break;
    default:
        return "None";
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

auto Util::GetText(Util::TEXT a_text) -> std::string
{
    return text[a_text];
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
    } else if (a_string == "dwemer") {
        return TYPE::Dwemer;
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

void Util::SetText(Util::TEXT a_text, std::string a_string)
{
    logs::info("Util::SetText :: Parsed text: '{}'", a_string);
    text.try_emplace(a_text, a_string);
}