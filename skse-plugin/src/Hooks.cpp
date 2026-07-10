#include "Hooks.h"
#include "Limits.h"
#include "Counter.h"
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <string>

namespace
{
	using UnknownCall_t = void(__cdecl*)();
	UnknownCall_t Original_FortifyAV = nullptr;

	void PatchTrainingUI()
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

		current += "/" + std::to_string(limit);

		if (count >= limit) {
			rt.trainerSkill.SetText("<font color='#FF4444'>" + current + "</font>");
		} else {
			rt.trainerSkill.SetText(current.c_str());
		}
	}

	class PatchTask : public SKSE::detail::TaskDelegate
	{
	public:
		void Run() override
		{
			auto* ui = RE::UI::GetSingleton();
			if (!ui) return;

			auto* menu = ui->GetMenu<RE::TrainingMenu>();
			if (!menu) return;

			PatchTrainingUI();

			auto* taskIntfc = SKSE::GetTaskInterface();
			if (taskIntfc) {
				taskIntfc->AddUITask(new PatchTask());
			}
		}

		void Dispose() override { delete this; }
	};

	class TrainingMenuWatcher : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
			RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
		{
			if (!a_event || a_event->menuName != "Training Menu") {
				return RE::BSEventNotifyControl::kContinue;
			}

			if (a_event->opening) {
				auto* taskIntfc = SKSE::GetTaskInterface();
				if (taskIntfc) {
					taskIntfc->AddUITask(new PatchTask());
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void InstallFortifyAVHook()
	{
		REL::Relocation<std::uintptr_t> target{ REL::ID(51793) };
		std::uintptr_t callAddr = target.address() + 0xCE;

		std::int32_t stoDisp = *reinterpret_cast<std::int32_t*>(callAddr + 1);
		std::uintptr_t stoThunk = callAddr + 5 + stoDisp;
		Original_FortifyAV = reinterpret_cast<UnknownCall_t>(stoThunk);

		auto* ourThunk = +[]() -> void
		{
			Original_FortifyAV();

			RE::ActorValue skill = RE::ActorValue::kNone;
			{
				auto* ui = RE::UI::GetSingleton();
				if (ui) {
					auto* menu = ui->GetMenu<RE::TrainingMenu>();
					if (menu) {
						skill = menu->GetRuntimeData().skill;
					}
				}
			}
			if (skill == RE::ActorValue::kNone) return;

			Counter::Increment(skill);

			int limit = Limits::GetLimit(skill);
			int count = Counter::GetCount(skill);
			SKSE::log::info("TM_TrainingLimit: {} ({}/{})",
				Limits::GetSkillName(skill), count, limit);
		};

		std::int32_t newDisp = static_cast<std::int32_t>(
			reinterpret_cast<std::uintptr_t>(ourThunk) - (callAddr + 5));
		REL::safe_write(callAddr + 1, std::span(&newDisp, 1));

		SKSE::log::info("TM_TrainingLimit: FortifyActorValue hook installed at REL::ID(51793)+0xCE");
	}

	TrainingMenuWatcher* g_watcher = nullptr;

	void InstallMenuWatcher()
	{
		g_watcher = new TrainingMenuWatcher();
		RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(g_watcher);
		SKSE::log::info("TM_TrainingLimit: MenuOpenCloseEvent watcher installed");
	}
}

void Hooks::Install()
{
	SKSE::log::info("TM_TrainingLimit: Installing hooks...");
	InstallFortifyAVHook();
	InstallMenuWatcher();
	SKSE::log::info("TM_TrainingLimit: Hooks installed.");
}
