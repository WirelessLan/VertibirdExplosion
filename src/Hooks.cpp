#include "Hooks.h"

namespace Hooks {
	template<std::uint64_t id, std::ptrdiff_t offset>
	class KillHook {
	public:
		static void Install(bool _playerVertibirdExplosion, bool _useVertibirdExplosionProbability, float _vertibirdExplosionProbability) {
			bPlayerVertibirdExplosion = _playerVertibirdExplosion;
			bUseVertibirdExplosionProbability = _useVertibirdExplosionProbability;
			fVertibirdExplosionProbability = _vertibirdExplosionProbability;

			REL::Relocation<uintptr_t> target(REL::ID(id), offset);
			origFunc = *(func_t*)(target.get());
			REL::safe_write(target.get(), (uintptr_t)Hook);
		}

	private:
		using func_t = void(*)(RE::Actor*, RE::Actor*, float, bool, bool);

		static bool IsVertibirdRace(RE::Actor* a_actor) {
			if (!a_actor || !a_actor->race)
				return false;
			
			if (a_actor->race->formID == 0x000D77E3)
				return true;

			return false;
		}

		static bool IsDeferredKillSet(RE::Actor* a_actor) {
			if (!a_actor || !a_actor->currentProcess || !a_actor->currentProcess->middleHigh)
				return false;

			if (a_actor->currentProcess->middleHigh->deferredKill > 0)
				return true;

			return false;
		}

		static void ClearDeferredKill(RE::Actor* a_actor) {
			if (!a_actor || !a_actor->currentProcess || !a_actor->currentProcess->middleHigh)
				return;

			a_actor->currentProcess->middleHigh->deferredKill = 0;
		}

		static bool IsPlayerInVertibird(RE::Actor* a_vert) {
			if (!a_vert)
				return false;

			RE::PlayerCharacter* pc = RE::PlayerCharacter::GetSingleton();
			if (!pc)
				return false;

			auto ref_ptr = pc->currentProcess->middleHigh->currentFurniture.get();
			if (!ref_ptr)
				return false;

			if (a_vert->formID == ref_ptr->formID)
				return true;

			return false;
		}

		static bool CheckProbability(float prob) {
			std::random_device rd;
			std::mt19937 gen(rd());

			std::uniform_int_distribution<uint32_t> dis(0, 100);
			uint32_t rand_prob = dis(gen);
			uint32_t uint_prob = static_cast<uint32_t>(prob * 100);

			if (rand_prob <= uint_prob)
				return true;

			return false;
		}

		static bool ShouldClearDeferredKill(RE::Actor* a_actor) {
			if (!bPlayerVertibirdExplosion && IsPlayerInVertibird(a_actor))
				return false;

			if (bUseVertibirdExplosionProbability && !CheckProbability(fVertibirdExplosionProbability))
				return false;

			return true;
		}

		static void Hook(RE::Actor* a_victim, RE::Actor* a_killer, float a_damage, bool a_sendEvent, bool a_ragdollInstant) {
			if (IsVertibirdRace(a_victim) && IsDeferredKillSet(a_victim) && ShouldClearDeferredKill(a_victim))
				ClearDeferredKill(a_victim);

			origFunc(a_victim, a_killer, a_damage, a_sendEvent, a_ragdollInstant);
		}

		inline static bool bPlayerVertibirdExplosion;
		inline static bool bUseVertibirdExplosionProbability;
		inline static float fVertibirdExplosionProbability;

		inline static func_t origFunc;
	};

	void Install(bool playerVertibirdExplosion, bool useVertibirdExplosionProbability, float vertibirdExplosionProbability) {
		KillHook<1455516, 0x8B8>::Install(playerVertibirdExplosion, useVertibirdExplosionProbability, vertibirdExplosionProbability);
	}
}
