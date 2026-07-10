#include "Counter.h"
#include "SKSE/SKSE.h"
#include <unordered_map>

namespace
{
	constexpr std::uint32_t SERIALIZATION_ID = 'TMTL';

	std::unordered_map<std::uint32_t, int> s_counts;
}

void Counter::Initialize()
{
	s_counts.clear();
}

int Counter::GetCount(RE::ActorValue a_skill)
{
	auto it = s_counts.find(static_cast<std::uint32_t>(a_skill));
	return (it != s_counts.end()) ? it->second : 0;
}

void Counter::Increment(RE::ActorValue a_skill)
{
	auto key = static_cast<std::uint32_t>(a_skill);
	auto it = s_counts.find(key);
	if (it != s_counts.end()) {
		++it->second;
	} else {
		s_counts[key] = 1;
	}
}

void Counter::OnGameSaved(SKSE::SerializationInterface* a_intfc)
{
	if (!a_intfc) return;

	if (!a_intfc->OpenRecord(SERIALIZATION_ID, 1)) {
		SKSE::log::error("Failed to open serialization record for save.");
		return;
	}

	std::uint32_t count = static_cast<std::uint32_t>(s_counts.size());
	if (!a_intfc->WriteRecordData(&count, sizeof(count))) {
		SKSE::log::error("Failed to write count header.");
		return;
	}

	for (const auto& [av, c] : s_counts) {
		if (!a_intfc->WriteRecordData(&av, sizeof(av))) return;
		if (!a_intfc->WriteRecordData(&c, sizeof(c))) return;
	}

	SKSE::log::debug("Serialized {} training counts.", count);
}

void Counter::OnGameLoaded(SKSE::SerializationInterface* a_intfc)
{
	if (!a_intfc) return;

	std::uint32_t type;
	std::uint32_t version;
	std::uint32_t length;

	s_counts.clear();

	while (a_intfc->GetNextRecordInfo(type, version, length)) {
		if (type != SERIALIZATION_ID) continue;

		std::uint32_t count = 0;
		if (length < sizeof(count)) continue;
		if (!a_intfc->ReadRecordData(&count, sizeof(count))) continue;

		for (std::uint32_t i = 0; i < count; ++i) {
			std::uint32_t av = 0;
			int c = 0;
			if (!a_intfc->ReadRecordData(&av, sizeof(av))) break;
			if (!a_intfc->ReadRecordData(&c, sizeof(c))) break;
			s_counts[av] = c;
		}

		SKSE::log::info("Loaded {} training counts from save.", count);
		break;
	}
}

void Counter::OnRevert(SKSE::SerializationInterface*)
{
	s_counts.clear();
	SKSE::log::info("Training counts cleared (new game / revert).");
}
