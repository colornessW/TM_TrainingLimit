#include "Limits.h"
#include "SKSE/SKSE.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace
{
	constexpr int DEFAULT_LIMIT = 20;

	struct SkillEntry
	{
		std::string   name;
		RE::ActorValue av;
	};

	const SkillEntry kSkillTable[] = {
		{ "OneHanded",   RE::ActorValue::kOneHanded },
		{ "TwoHanded",   RE::ActorValue::kTwoHanded },
		{ "Archery",     RE::ActorValue::kArchery },
		{ "Block",       RE::ActorValue::kBlock },
		{ "Smithing",    RE::ActorValue::kSmithing },
		{ "HeavyArmor",  RE::ActorValue::kHeavyArmor },
		{ "LightArmor",  RE::ActorValue::kLightArmor },
		{ "Pickpocket",  RE::ActorValue::kPickpocket },
		{ "Lockpicking", RE::ActorValue::kLockpicking },
		{ "Sneak",       RE::ActorValue::kSneak },
		{ "Alchemy",     RE::ActorValue::kAlchemy },
		{ "Speech",      RE::ActorValue::kSpeech },
		{ "Alteration",  RE::ActorValue::kAlteration },
		{ "Conjuration", RE::ActorValue::kConjuration },
		{ "Destruction", RE::ActorValue::kDestruction },
		{ "Illusion",    RE::ActorValue::kIllusion },
		{ "Restoration", RE::ActorValue::kRestoration },
		{ "Enchanting",  RE::ActorValue::kEnchanting },
	};

	std::unordered_map<RE::ActorValue, int> s_limits;
	std::unordered_map<RE::ActorValue, std::string> s_names;
	std::unordered_map<std::string, RE::ActorValue> s_nameToAV;
	int s_defaultLimit = DEFAULT_LIMIT;

	std::string ToLower(std::string a_str)
	{
		for (auto& c : a_str) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		return a_str;
	}
}

void Limits::Load()
{
	for (const auto& entry : kSkillTable) {
		s_names[entry.av] = entry.name;
		s_nameToAV[ToLower(entry.name)] = entry.av;
		s_limits[entry.av] = DEFAULT_LIMIT;
	}

	struct ParsedOverride { RE::ActorValue av; int limit; };
	std::vector<ParsedOverride> overrides;
	int parsedDefault = DEFAULT_LIMIT;

	static std::filesystem::path s_iniPath;
	if (s_iniPath.empty()) {
		HMODULE hModule = nullptr;
		static int dummy = 0;
		GetModuleHandleExA(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCSTR>(&dummy),
			&hModule);
		char buf[MAX_PATH];
		GetModuleFileNameA(hModule, buf, MAX_PATH);
		s_iniPath = std::filesystem::path(buf).parent_path() / "TM_TrainingLimit.ini";
	}

	if (!std::filesystem::exists(s_iniPath)) {
		SKSE::log::info("TM_TrainingLimit: INI not found at {}, using defaults ({} per skill).",
			s_iniPath.string(), DEFAULT_LIMIT);
		return;
	}

	std::ifstream file(s_iniPath);
	if (!file.is_open()) {
		SKSE::log::warn("TM_TrainingLimit: failed to open INI at {}", s_iniPath.string());
		return;
	}

	std::string line;
	std::string section;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == ';' || line[0] == '#') continue;

		if (line[0] == '[') {
			auto end = line.find(']');
			if (end != std::string::npos) {
				section = line.substr(1, end - 1);
			}
			continue;
		}

		if (section != "Limits") continue;

		auto eq = line.find('=');
		if (eq == std::string::npos) continue;

		auto key = line.substr(0, eq);
		auto val = line.substr(eq + 1);

		while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();
		while (!key.empty() && (key.front() == ' ' || key.front() == '\t')) key = key.substr(1);
		while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) val.pop_back();
		while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val = val.substr(1);

		if (key.empty() || val.empty()) continue;

		int limit = std::atoi(val.c_str());
		if (limit <= 0) limit = DEFAULT_LIMIT;

		if (ToLower(key) == "idefault") {
			parsedDefault = limit;
		} else if (key.size() > 1 && key[0] == 'i') {
			auto skillName = key.substr(1);
			auto it = s_nameToAV.find(ToLower(skillName));
			if (it != s_nameToAV.end()) {
				overrides.push_back({ it->second, limit });
			}
		}
	}

	file.close();

	s_defaultLimit = parsedDefault;
	for (auto& [av, limit] : s_limits) {
		limit = parsedDefault;
	}
	for (const auto& ov : overrides) {
		s_limits[ov.av] = ov.limit;
	}

	for (const auto& [av, limit] : s_limits) {
		auto nameIt = s_names.find(av);
		if (nameIt != s_names.end()) {
			SKSE::log::info("TM_TrainingLimit: {} limit = {}", nameIt->second, limit);
		}
	}
}

int Limits::GetDefault()
{
	return s_defaultLimit;
}

int Limits::GetLimit(RE::ActorValue a_skill)
{
	auto it = s_limits.find(a_skill);
	return (it != s_limits.end()) ? it->second : s_defaultLimit;
}

const char* Limits::GetSkillName(RE::ActorValue a_skill)
{
	auto it = s_names.find(a_skill);
	return (it != s_names.end()) ? it->second.c_str() : "Unknown";
}

RE::ActorValue Limits::GetSkillByName(const std::string& a_name)
{
	auto it = s_nameToAV.find(ToLower(a_name));
	return (it != s_nameToAV.end()) ? it->second : RE::ActorValue::kNone;
}
