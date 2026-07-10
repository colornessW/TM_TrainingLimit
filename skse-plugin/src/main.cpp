#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "Hooks.h"
#include "Limits.h"
#include "Counter.h"

namespace
{
	void OnSerialization(SKSE::SerializationInterface* a_intfc)
	{
		Counter::OnGameSaved(a_intfc);
	}

	void OnDeserialization(SKSE::SerializationInterface* a_intfc)
	{
		Counter::OnGameLoaded(a_intfc);
	}

	void OnRevert(SKSE::SerializationInterface* a_intfc)
	{
		Counter::OnRevert(a_intfc);
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (!a_msg) return;

		switch (a_msg->type) {
		case SKSE::MessagingInterface::kDataLoaded:
			SKSE::log::info("TM_TrainingLimit: Data loaded.");
			break;
		case SKSE::MessagingInterface::kPostLoadGame:
			SKSE::log::info("TM_TrainingLimit: Game loaded.");
			break;
		case SKSE::MessagingInterface::kNewGame:
			SKSE::log::info("TM_TrainingLimit: New game started.");
			Counter::OnRevert(nullptr);
			break;
		default:
			break;
		}
	}
}

SKSEPluginInfo(
	.Version = { 1, 0, 0, 0 },
	.Name = "TM_TrainingLimit",
	.Author = "Thu'mundus",
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	SKSE::log::info("TM_TrainingLimit v1.0.0 loading...");

	Limits::Load();
	Counter::Initialize();

	auto* serialization = SKSE::GetSerializationInterface();
	if (serialization) {
		serialization->SetUniqueID('TMTL');
		serialization->SetSaveCallback(OnSerialization);
		serialization->SetLoadCallback(OnDeserialization);
		serialization->SetRevertCallback(OnRevert);
		SKSE::log::info("TM_TrainingLimit: Serialization registered.");
	} else {
		SKSE::log::warn("TM_TrainingLimit: Serialization interface unavailable.");
	}

	auto* messaging = SKSE::GetMessagingInterface();
	if (messaging) {
		messaging->RegisterListener(OnMessage);
		SKSE::log::info("TM_TrainingLimit: Messaging registered.");
	}

	Hooks::Install();

	SKSE::log::info("TM_TrainingLimit v1.0.0 loaded.");
	return true;
}
