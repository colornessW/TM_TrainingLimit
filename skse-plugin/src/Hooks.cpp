#include "Hooks.h"
#include "Limits.h"
#include "Counter.h"
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <memory>
#include <string>

namespace
{
	using UnknownCall_t = void(__cdecl*)();
	UnknownCall_t Original_FortifyAV = nullptr;

	auto g_poller = std::make_shared<std::function<void()>>();

	void PatchTrainingUI()
	{
		auto* ui = RE::UI::GetSingleton();
		if (!ui) return;

		auto menu = ui->GetMenu<RE::TrainingMenu>();
		if (!menu) return;

		auto& rt = menu->GetRuntimeData();

		std::string current = rt.trainerSkill.GetString();
		if (current.empty()) return;

		if (current.find('/') != std::string::npos) return;

		int limit = Limits::GetLimit(rt.skill);
		int count = Counter::GetCount(rt.skill);

		current += "/" + std::to_string(limit);

		if (count >= limit) {
			rt.trainerSkill.SetTextHTML(("<font color='#FF4444'>" + current + "</font>").c_str());
		} else {
			rt.trainerSkill.SetText(current.c_str());
		}
	}

	void StartUIPolling()
	{
		*g_poller = [p = g_poller]()
		{
			auto* ui = RE::UI::GetSingleton();
			if (!ui || !ui->GetMenu<RE::TrainingMenu>()) return;

			PatchTrainingUI();

			auto* taskIntfc = SKSE::GetTaskInterface();
			if (taskIntfc) {
				taskIntfc->AddUITask(*p);
			}
		};

		auto* taskIntfc = SKSE::GetTaskInterface();
		if (taskIntfc) {
			taskIntfc->AddUITask(*g_poller);
		}
	}

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
				StartUIPolling();
			}

			return RE::BSEventNotifyControl::kContinue;
		}
	};

	TrainingMenuWatcher* g_watcher = nullptr;

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
					auto menu = ui->GetMenu<RE::TrainingMenu>();
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
