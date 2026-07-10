#pragma once

#include <cstdint>
#include <unordered_map>
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

namespace Counter
{
	void Initialize();

	int GetCount(RE::ActorValue a_skill);
	void Increment(RE::ActorValue a_skill);

	void OnGameSaved(SKSE::SerializationInterface* a_intfc);
	void OnGameLoaded(SKSE::SerializationInterface* a_intfc);
	void OnRevert(SKSE::SerializationInterface* a_intfc);
}
