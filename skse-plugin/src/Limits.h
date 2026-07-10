#pragma once

#include <string>
#include <unordered_map>
#include "RE/Skyrim.h"

namespace Limits
{
	void Load();

	int GetDefault();
	int GetLimit(RE::ActorValue a_skill);

	const char* GetSkillName(RE::ActorValue a_skill);
	RE::ActorValue GetSkillByName(const std::string& a_name);
}
