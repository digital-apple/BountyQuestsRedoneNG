#include "System.h"
#include "Events.h"
#include "Papyrus.h"
#include "Serialization.h"

void InitializeLogger()
{
    auto path = SKSE::log::log_directory();

    if (!path) { return; }

    const auto plugin = SKSE::PluginDeclaration::GetSingleton();
    *path /= std::format("{}.log", plugin->GetName());

    std::vector<spdlog::sink_ptr> sinks{
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true),
        std::make_shared<spdlog::sinks::msvc_sink_mt>()
    };

    auto logger = std::make_shared<spdlog::logger>("global", sinks.begin(), sinks.end());

    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(logger));
    spdlog::set_pattern("[%^%L%$] %v");
}

void HandleMessage(SKSE::MessagingInterface::Message* a_message)
{
    switch (a_message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        {
            Events::GetSingleton()->Register();
            const auto system = System::GetSingleton();

            system->ParseQuests();
            system->ParseRewards();
            system->ParseTrackers();
            system->ParseTexts();

            break;
        }
    }
}

void InitializePapyrus()
{
    TRACE("Initializing papyrus bindings...");

    if (SKSE::GetPapyrusInterface()->Register(Papyrus::RegisterFunctions)) {
        INFO("Papyrus functions bound.");
    } else {
        stl::report_and_fail("Failure to register Papyrus bindings.");
    }
}

void InitializeSerialization()
{
    TRACE("Initializing cosave serialization...");

    auto* serialization_interface = SKSE::GetSerializationInterface();

    serialization_interface->SetUniqueID('BQNG');
    serialization_interface->SetSaveCallback(Serialization::OnGameSaved);
    serialization_interface->SetRevertCallback(Serialization::OnRevert);
    serialization_interface->SetLoadCallback(Serialization::OnGameLoaded);

    TRACE("Cosave serialization initialized.");
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    InitializeLogger();

    const auto plugin = SKSE::PluginDeclaration::GetSingleton();
    INFO("{} v{} is loading...", plugin->GetName(), plugin->GetVersion());

    SKSE::Init(a_skse);

    const auto messaging_interface = SKSE::GetMessagingInterface();

    if (!messaging_interface) { stl::report_and_fail("Failed to communicate with the messaging interface!"); }

    messaging_interface->RegisterListener(HandleMessage);

    InitializePapyrus();
    InitializeSerialization();

    INFO("{} loaded.", plugin->GetName());

    return true;
}
