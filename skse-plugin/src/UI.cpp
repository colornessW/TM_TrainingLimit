#include "UI.h"
#include "Limits.h"
#include "Counter.h"
#include "RE/Skyrim.h"

void UI::PatchTrainingMenuText()
{
	auto* ui = RE::UI::GetSingleton();
	if (!ui) return;

	auto* menu = ui->GetMenu<RE::TrainingMenu>();
	if (!menu) return;

	auto& rt = menu->GetRuntimeData();

	std::string current = rt.trainerSkill.GetString();
	if (current.empty()) return;

	if (current.find('/') != std::string::npos) return;

	int limit = Limits::GetLimit(rt.skill);
	int count = Counter::GetCount(rt.skill);

	std::string modified = current + "/" + std::to_string(limit);
	if (count >= limit) {
		modified += " (MAX)";
	}

	rt.trainerSkill.SetText(modified.c_str());
}
