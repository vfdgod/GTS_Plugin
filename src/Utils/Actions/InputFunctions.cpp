#include "Utils/Actions/InputFunctions.hpp"

#include "Config/Config.hpp"

#include "Utils/Actions/InputConditions.hpp"
#include "Utils/Actions/TotalControlActions.hpp"

#include "Managers/Animation/Utils/AnimationUtils.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Input/InputManager.hpp"
#include "Managers/Animation/Controllers/VoreController.hpp"
#include "Managers/Rumble.hpp"
#include "Managers/HighHeel.hpp"
#include "Managers/AttributeManager.hpp"
#include "Managers/GTSSizeManager.hpp"
#include "Managers/MaxSizeManager.hpp"
#include "Managers/Audio/MoansLaughs.hpp"

#include "Magic/Effects/Common.hpp"
#include "Utils/Actor/ActorUtils.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

using namespace GTS;

namespace {

	constexpr int StruggleMax = 40;
	constexpr float RECALL_SHRINK_EPSILON = 0.001f;
	constexpr float RECALL_SEARCH_RADIUS_MIN = 250.0f;
	constexpr float RECALL_SEARCH_RADIUS_MAX = 20000.0f;
	constexpr float RECALL_PAUSE_MIN = 0.0f;
	constexpr float RECALL_PAUSE_MAX = 10.0f;
	constexpr float RECALL_PLAYER_DISTANCE_MIN = 60.0f;
	constexpr float RECALL_PLAYER_DISTANCE_MAX = 2000.0f;
	constexpr float RECALL_ACTOR_SPACING_MIN = 40.0f;
	constexpr float RECALL_ACTOR_SPACING_MAX = 1000.0f;
	constexpr float RECALL_AUTO_INTERVAL_MIN = 1.0f;
	constexpr float RECALL_AUTO_INTERVAL_MAX = 600.0f;
	constexpr float RECALL_NOTIFY_INTERVAL_MIN = 1.0f;
	constexpr float RECALL_NOTIFY_INTERVAL_MAX = 600.0f;
	constexpr int RECALL_FRONT_PER_ROW = 5;

	template <class Enum>
	bool TryParseEnumString(const std::string& a_value, Enum& a_out) {
		if (auto parsed = magic_enum::enum_cast<Enum>(a_value); parsed.has_value()) {
			a_out = *parsed;
			return true;
		}
		return false;
	}

	LShrinkRecallFilterMode_t GetRecallFilterMode() {
		LShrinkRecallFilterMode_t mode = LShrinkRecallFilterMode_t::kAllShrunken;
		TryParseEnumString(Config::Balance.sShrinkRecallFilterMode, mode);
		return mode;
	}

	LShrinkRecallPlacement_t GetRecallPlacementMode() {
		LShrinkRecallPlacement_t mode = LShrinkRecallPlacement_t::kRing;
		TryParseEnumString(Config::Balance.sShrinkRecallPlacement, mode);
		return mode;
	}

	float GetRecallSearchRadius() {
		return std::clamp(Config::Balance.fShrinkRecallSearchRadius, RECALL_SEARCH_RADIUS_MIN, RECALL_SEARCH_RADIUS_MAX);
	}

	float GetRecallPauseDuration() {
		return std::clamp(Config::Balance.fShrinkRecallPauseDuration, RECALL_PAUSE_MIN, RECALL_PAUSE_MAX);
	}

	float GetRecallPlayerDistance() {
		return std::clamp(Config::Balance.fShrinkRecallPlayerDistance, RECALL_PLAYER_DISTANCE_MIN, RECALL_PLAYER_DISTANCE_MAX);
	}

	float GetRecallActorSpacing() {
		return std::clamp(Config::Balance.fShrinkRecallActorSpacing, RECALL_ACTOR_SPACING_MIN, RECALL_ACTOR_SPACING_MAX);
	}

	float GetRecallAutoInterval() {
		return std::clamp(Config::Balance.fShrinkRecallAutoInterval, RECALL_AUTO_INTERVAL_MIN, RECALL_AUTO_INTERVAL_MAX);
	}

	float GetRecallNotifyInterval() {
		return std::clamp(Config::Balance.fShrinkRecallNotifyInterval, RECALL_NOTIFY_INTERVAL_MIN, RECALL_NOTIFY_INTERVAL_MAX);
	}

	bool IsShrunkenActor(Actor* actor) {
		if (!actor) {
			return false;
		}
		const float naturalScale = get_natural_scale(actor, true);
		const float currentScale = get_visual_scale(actor);
		return currentScale < naturalScale - RECALL_SHRINK_EPSILON;
	}

	bool MatchesRecallCustomTargets(Actor* actor) {
		for (const auto& targetName : Config::Balance.ShrinkRecallTargets) {
			LSizeLimitRuleTarget_t target = LSizeLimitRuleTarget_t::kHumanoidNPC;
			if (!TryParseEnumString(targetName, target) || target == LSizeLimitRuleTarget_t::kPlayer) {
				continue;
			}

			if (ActorMatchesSizeLimitRuleTarget(actor, target)) {
				return true;
			}
		}

		return false;
	}

	bool ShouldRecallActor(Actor* actor, const NiPoint3& playerPosition, float searchRadiusSquared, bool useCustomTargets) {
		if (!actor || actor->IsPlayerRef()) {
			return false;
		}
		if (actor->IsDead() || !actor->Is3DLoaded()) {
			return false;
		}

		const NiPoint3 distance = actor->GetPosition() - playerPosition;
		if (distance.x * distance.x + distance.y * distance.y + distance.z * distance.z > searchRadiusSquared) {
			return false;
		}

		auto transient = Transient::GetActorData(actor);
		if (transient) {
			if (transient->BeingHeld || transient->BetweenBreasts || transient->AboutToBeEaten || transient->ReattachingTiny) {
				return false;
			}
		}

		if (AnimationVars::General::IsGTSBusy(actor) || AnimationVars::General::IsTransitioning(actor)) {
			return false;
		}
		if (!IsShrunkenActor(actor)) {
			return false;
		}

		if (useCustomTargets) {
			return MatchesRecallCustomTargets(actor);
		}

		return true;
	}

	RE::NiPoint3 GetPlayerForwardVector(Actor* player) {
		RE::NiPoint3 forwardVector{ 0.0f, 1.0f, 0.0f };
		RE::NiPoint3 forward = RotateAngleAxis(forwardVector, -player->data.angle.z, { 0.0f, 0.0f, 1.0f });
		const float length = forward.Length();
		if (length <= 0.0001f) {
			return RE::NiPoint3{ 0.0f, 1.0f, 0.0f };
		}
		return forward / length;
	}

	RE::NiPoint3 GetPlayerRightVector(Actor* player) {
		RE::NiPoint3 rightVector{ 1.0f, 0.0f, 0.0f };
		RE::NiPoint3 right = RotateAngleAxis(rightVector, -player->data.angle.z, { 0.0f, 0.0f, 1.0f });
		const float length = right.Length();
		if (length <= 0.0001f) {
			return RE::NiPoint3{ 1.0f, 0.0f, 0.0f };
		}
		return right / length;
	}

	std::vector<Actor*> CollectRecallActors(Actor* player, float searchRadius) {
		std::vector<Actor*> recalledActors;
		recalledActors.reserve(16);
		const NiPoint3 playerPosition = player->GetPosition();
		const float searchRadiusSquared = searchRadius * searchRadius;
		const bool useCustomTargets = GetRecallFilterMode() == LShrinkRecallFilterMode_t::kCustomTargets;

		for (auto actor : find_actors()) {
			if (!ShouldRecallActor(actor, playerPosition, searchRadiusSquared, useCustomTargets)) {
				continue;
			}
			recalledActors.push_back(actor);
		}

		return recalledActors;
	}

	RE::NiPoint3 GetRecallPlacementOffset(std::size_t index, LShrinkRecallPlacement_t placement, const RE::NiPoint3& forward, const RE::NiPoint3& right) {
		const float playerDistance = GetRecallPlayerDistance();
		const float actorSpacing = GetRecallActorSpacing();

		if (placement == LShrinkRecallPlacement_t::kFront) {
			const int row = static_cast<int>(index) / RECALL_FRONT_PER_ROW;
			const int column = static_cast<int>(index) % RECALL_FRONT_PER_ROW;
			const float centeredColumn = static_cast<float>(column) - (static_cast<float>(RECALL_FRONT_PER_ROW - 1) * 0.5f);

			return forward * (playerDistance + row * actorSpacing) +
				right * (centeredColumn * actorSpacing);
		}

		std::size_t remaining = index;
		int layer = 0;
		while (true) {
			const float radius = playerDistance + layer * actorSpacing;
			const int capacity = std::max(6, static_cast<int>(std::floor((2.0f * std::numbers::pi_v<float> * radius) / actorSpacing)));
			if (remaining < static_cast<std::size_t>(capacity)) {
				const float angle = (2.0f * std::numbers::pi_v<float> * static_cast<float>(remaining)) / static_cast<float>(capacity);
				return forward * (std::cos(angle) * radius) + right * (std::sin(angle) * radius);
			}

			remaining -= static_cast<std::size_t>(capacity);
			layer += 1;
		}
	}

	void StartRecallPause(Actor* actor, float duration) {
		if (!actor || duration <= RECALL_PAUSE_MIN) {
			return;
		}

		if (auto transient = Transient::GetActorData(actor)) {
			transient->RecallPauseTimer.UpdateDelta(duration);
			transient->RecallPauseTimer.ResetGate();
		}
	}

	void MoveRecallActors(Actor* player, std::vector<Actor*>& recalledActors, float pauseDuration) {
		const auto placement = GetRecallPlacementMode();
		const RE::NiPoint3 playerPos = player->GetPosition();
		const RE::NiPoint3 forward = GetPlayerForwardVector(player);
		const RE::NiPoint3 right = GetPlayerRightVector(player);

		std::ranges::sort(recalledActors, [&](Actor* lhs, Actor* rhs) {
			return lhs->GetPosition().GetDistance(playerPos) < rhs->GetPosition().GetDistance(playerPos);
		});

		for (std::size_t index = 0; index < recalledActors.size(); ++index) {
			Actor* actor = recalledActors[index];
			RE::NiPoint3 destination = playerPos + GetRecallPlacementOffset(index, placement, forward, right);
			destination.z = playerPos.z;
			actor->SetPosition(destination, true);
			StartRecallPause(actor, pauseDuration);
		}
	}

	std::size_t RecallShrunkenActors(bool notifyEmpty, bool notifySuccess) {
		Actor* player = PlayerCharacter::GetSingleton();
		if (!player) {
			return 0;
		}

		const float searchRadius = GetRecallSearchRadius();
		const float pauseDuration = GetRecallPauseDuration();
		std::vector<Actor*> recalledActors = CollectRecallActors(player, searchRadius);

		if (recalledActors.empty()) {
			if (notifyEmpty) {
				Notify("附近没有符合条件的缩小目标");
			}
			return 0;
		}

		MoveRecallActors(player, recalledActors, pauseDuration);

		if (notifySuccess) {
			if (pauseDuration > RECALL_PAUSE_MIN) {
				Notify("已移动 {} 个缩小目标，并停步 {:.1f} 秒", recalledActors.size(), pauseDuration);
			}
			else {
				Notify("已移动 {} 个缩小目标", recalledActors.size());
			}
		}

		return recalledActors.size();
	}

	void NotifyNearbyShrunkenActors() {
		Actor* player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		const std::size_t count = CollectRecallActors(player, GetRecallSearchRadius()).size();
		if (count > 0) {
			Notify("附近有 {} 个符合条件的缩小目标", count);
		}
	}

	void ResetEscapeDataTask() {
		std::string name = std::format("ResetStruggle_{}", Time::WorldTimeElapsed());
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		ActorHandle gianthandle = player->CreateRefHandle();
		double Start = Time::WorldTimeElapsed();

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			if (!giantref) {
				return false;
			}
			double Finish = Time::WorldTimeElapsed();
			double timepassed = Finish - Start;
			if (timepassed > 0.25f) {
				auto transient = Transient::GetActorData(giantref);
				if (transient) {
					transient->EscapingInteraction = false;
					transient->EscapingActionProgress = 0.0f;
					logger::info("Reset Transient Data successfully");
				}
				return false;
			}
			return true;
		});
	}

	void ReportScaleIntoConsole(Actor* actor, bool enemy) {
		if (actor && !actor->IsDead()) {
			float hh = HighHeelManager::GetBaseHHOffset(actor)[2]/100;
			float gigantism = Ench_Aspect_GetPower(actor) * 100;
			float naturalscale = get_natural_scale(actor, true);
			float scale = get_visual_scale(actor);
			float maxscale = get_max_scale(actor);

			Actor* player = PlayerCharacter::GetSingleton();
			if (!player) {
				return;
			}

			float BB = GetSizeFromBoundingBox(actor);
			if (enemy) {

				Cprint("{} Bounding Box To Size: {:.2f}, GameScale: {:.2f}", 
					actor->GetDisplayFullName(), 
					BB, 
					game_getactorscale(actor)
				);

				Cprint("{} Size Difference With the Player: {:.2f}", 
					actor->GetDisplayFullName(), 
					get_scale_difference(player, actor, SizeType::VisualScale, false, true)
				);
			}
			else {
				Cprint("{} Height: {} Weight: {}", 
					actor->GetDisplayFullName(),
					GetFormatedHeight(actor),
					GetFormatedWeight(actor)
				);
			}

			if (maxscale > 2500.0f) {

				Cprint("{} Scale: {:.2f}  (Natural Scale: {:.2f}; Bounding Box: {}; Size Limit: Infinite; Aspect Of Giantess: {:.1f}%)", 
					actor->GetDisplayFullName(), 
					scale, 
					naturalscale, 
					BB, 
					gigantism
				);
			}
			else {
				Cprint("{} Scale: {:.2f}  (Natural Scale: {:.2f}; Bounding Box: {}; Size Limit: {:.2f}; Aspect Of Giantess: {:.1f}%)", 
					actor->GetDisplayFullName(), 
					scale, 
					naturalscale, 
					BB, 
					maxscale, 
					gigantism
				);
			}

			if (hh > 0.0f) { // if HH is > 0, print HH info
				Cprint("{} High Heels: {:.2f} (+{:.2f} cm / +{:.2f} ft)", 
					actor->GetDisplayFullName(), 
					hh, 
					hh, 
					hh*3.28f
				);
			}
		}
	}

	void regenerate_health(Actor* giant, float value) {
		if (Runtime::HasPerk(giant, Runtime::PERK.GTSPerkSizeReserveAug2)) {
			float maxhp = GetMaxAV(giant, ActorValue::kHealth);
			float regenerate = maxhp * 0.25f * value; // 25% of health

			giant->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, regenerate * TimeScale());
		}
	}

	//////////////////////////////////////////////////////////////////// Growths/Shrinks that have duration but no animation, we forgot to add them

	void TotalControlEnlargeTeammate_OverTime(const ManagedInputEvent&) {
		TotalControlActions::GrowTeammatesOverTime(1.0f);
	}

	void TotalControlShrinkTeammate_OverTime(const ManagedInputEvent&) {
		TotalControlActions::ShrinkTeammatesOverTime(1.0f);
	}

	void TotalControlGrowPlayer_OverTime(const ManagedInputEvent&) {
		TotalControlActions::GrowPlayerOverTime(1.0f);
	}

	void TotalControlShrinkPlayer_OverTime(const ManagedInputEvent&) {
		TotalControlActions::ShrinkPlayerOverTime(1.0f, Minimum_Actor_Scale * 2.0f);
	}

	////////////////////////////////////////////////////////////////////

	void TotalControlGrowEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) return;
		float scale = get_visual_scale(player);
		float stamina = std::clamp(GetStaminaPercentage(player), 0.05f, 1.0f);

		float perk = Perk_GetCostReduction(player);

		DamageAV(player, ActorValue::kStamina, 0.15f * perk * (scale * 0.5f + 0.5f) * stamina * TimeScale());
		Grow(player, 0.0010f * stamina, 0.0f);
		float Volume = std::clamp(get_visual_scale(player)/16.0f, 0.20f, 2.0f);
		Rumbling::Once("ColossalGrowth", player, 0.15f, 0.05f);
		static Timer timergrowth = Timer(2.00);
		if (timergrowth.ShouldRun()) {
			Runtime::PlaySoundAtNode(Runtime::SNDR.GTSSoundGrowth, player, Volume, "NPC Pelvis [Pelv]");
		}
	}

	void TotalControlShrinkEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) return;
		float scale = get_visual_scale(player);
		float stamina = std::clamp(GetStaminaPercentage(player), 0.05f, 1.0f);

		float perk = Perk_GetCostReduction(player);

		if (get_target_scale(player) > 0.12f) {
			DamageAV(player, ActorValue::kStamina, 0.07f * perk * (scale * 0.5f + 0.5f) * stamina * TimeScale());
			ShrinkActor(player, 0.0010f * stamina, 0.0f);
		} else {
			set_target_scale(player, 0.12f);
		}

		float Volume =std::clamp(get_visual_scale(player)*0.10f, 0.10f, 1.0f);
		Rumbling::Once("ColossalGrowth", player, 0.15f, 0.05f);
		static Timer timergrowth = Timer(2.00);
		if (timergrowth.ShouldRun()) {
			Runtime::PlaySound(Runtime::SNDR.GTSSoundShrink, player, Volume, 1.0f);
		}
	}

	void TotalControlGrowOtherEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) return;
		for (auto actor: find_actors()) {
			if (!actor) {
				continue;
			}
			if (!actor->IsPlayerRef() && (IsTeammate(actor))) {

				float perk = Perk_GetCostReduction(player);

				float npcscale = get_visual_scale(actor);
				float magicka = std::clamp(GetMagikaPercentage(player), 0.05f, 1.0f);
				DamageAV(player, ActorValue::kMagicka, 0.15f * perk * (npcscale * 0.5f + 0.5f) * magicka * TimeScale());
				Grow(actor, 0.0010f * magicka, 0.0f);
				float Volume = std::clamp(get_visual_scale(actor) / 16.0f, 0.20f, 2.0f);
				Rumbling::Once("TotalControlOther", actor, 0.15f, 0.05f);
				static Timer timergrowth = Timer(2.00);
				if (timergrowth.ShouldRun()) {
					Runtime::PlaySoundAtNode(Runtime::SNDR.GTSSoundGrowth, actor, Volume, "NPC Pelvis [Pelv]");
				}
			}
		}
	}

	void TotalControlShrinkOtherEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) return;
		for (auto actor: find_actors()) {
			if (!actor) {
				continue;
			}
			if (!actor->IsPlayerRef() && (IsTeammate(actor))) {
				
				float perk = Perk_GetCostReduction(player);

				float npcscale = get_visual_scale(actor);
				float magicka = std::clamp(GetMagikaPercentage(player), 0.05f, 1.0f);
				DamageAV(player, ActorValue::kMagicka, 0.07f * perk * (npcscale * 0.5f + 0.5f) * magicka * TimeScale());
				ShrinkActor(actor, 0.0010f * magicka, 0.0f);
				float Volume = std::clamp(get_visual_scale(actor) * 0.10f, 0.10f, 1.0f);
				Rumbling::Once("TotalControlOther", actor, 0.15f, 0.05f);
				static Timer timergrowth = Timer(2.00);
				if (timergrowth.ShouldRun()) {
					Runtime::PlaySound(Runtime::SNDR.GTSSoundShrink, actor, Volume, 1.0f);
				}
			} 
		}
	}

	////////////////////////////////////////////////////////////////////

	void RapidGrowthEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) return;
		float target = get_target_scale(player);
		float max_scale = get_max_scale(player);
		if (target >= max_scale) {
			NotifyWithSound(player, "已经无法再继续变大了");
			Rumbling::Once("CantGrow", player, 0.25f, 0.05f);
			return;
		}
		AnimationManager::StartAnim("TriggerGrowth", player);
	}

	void RapidShrinkEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) return;
		float target = get_target_scale(player);
		if (target <= Minimum_Actor_Scale) {
			NotifyWithSound(player, "已经无法再继续缩小了");
			Rumbling::Once("CantGrow", player, 0.25f, 0.05f);
			return;
		}
		AnimationManager::StartAnim("TriggerShrink", player);
	}

	////////////////////////////////////////////////////////////////////

	void SizeReserveEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		auto cache = Persistent::GetActorData(player);
		if (!cache || cache->fSizeReserve <= 0.0f || AnimationVars::Grab::IsGrabAttacking(player)) {
			return;
		}

		const float duration = data.Duration();
		if (duration < 1.2f || !Runtime::HasPerk(player, Runtime::PERK.GTSPerkSizeReserve)) {
			return;
		}
		if ((get_target_scale(player) >= 1.49f && TinyCalamityActive(player)) || Grab::GetHeldActor(player)) {
			return;
		}

		const float sizeCalculation = duration - 1.2f;
		const float gigantism = 1.0f + Ench_Aspect_GetPower(player);
		static Timer growthTimer = Timer(3.00);
		if (growthTimer.ShouldRunFrame()) {
			Runtime::PlaySoundAtNode(Runtime::SNDR.GTSSoundGrowth, player, cache->fSizeReserve / 50 * duration, "NPC Pelvis [Pelv]");
			Task_FacialEmotionTask_Moan(player, 2.0f, "SizeReserve");
			Sound_PlayMoans(player, 0.8f, 0.14f, EmotionTriggerSource::Growth);
		}

		const float shakePower = std::clamp(cache->fSizeReserve / 15 * duration, 0.0f, 2.0f);
		Rumbling::Once("SizeReserve", player, shakePower, 0.05f);

		const float reserveUsed = sizeCalculation / 80;
		update_target_scale(player, reserveUsed * gigantism, SizeEffectType::kNeutral);
		regenerate_health(player, reserveUsed * gigantism);
		cache->fSizeReserve = std::max(0.0f, cache->fSizeReserve - reserveUsed);
	}

	void DisplaySizeReserveEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player || !Runtime::HasPerk(player, Runtime::PERK.GTSPerkSizeReserve)) {
			return;
		}

		if (auto cache = Persistent::GetActorData(player)) {
			const float gigantism = 1.0f + Ench_Aspect_GetPower(player);
			Notify("体型储备：{:.2f}", cache->fSizeReserve * gigantism);
		}
	}

	void DebugReportEvent(const ManagedInputEvent& data) { // Report enemy scale into console
		for (auto actor : find_actors()) {
			if (actor && !actor->IsPlayerRef()) {
				ReportScaleIntoConsole(actor, !IsTeammate(actor));
			}
		}
	}

	void ShrinkOutburstEvent(const ManagedInputEvent& data) {

		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		bool DarkArts2 = Runtime::HasPerk(player, Runtime::PERK.GTSPerkDarkArtsAug2);
		bool DarkArts3 = Runtime::HasPerk(player, Runtime::PERK.GTSPerkDarkArtsAug3);
		bool DarkArts_Legendary = Runtime::HasPerk(player, Runtime::PERK.GTSPerkDarkArtsLegendary);

		float gigantism = 1.0f + Ench_Aspect_GetPower(player);

		float multi = AttributeManager::GetAttributeBonus(player, ActorValue::kHealth);

		float healthCur = GetAV(player, ActorValue::kHealth);
		float damagehp = 80.0f;

		if (DarkArts2) {
			damagehp -= 10; // less hp drain
		}
		if (DarkArts3) {
			damagehp -= 10; // even less hp drain
		}
		if (DarkArts_Legendary) {
			damagehp -= 12; // even more hp decrease
		}

		damagehp *= multi;
		damagehp /= gigantism;

		if (healthCur < damagehp * 1.10f) {
			Notify("生命值过低！");
			return; // don't allow us to die from own shrinking
		}

		static Timer NotifyTimer = Timer(2.0);
		bool OnCooldown = IsActionOnCooldown(player, CooldownSource::Misc_ShrinkOutburst);
		if (OnCooldown) {
			if (NotifyTimer.ShouldRunFrame()) {
				double cooldown = GetRemainingCooldown(player, CooldownSource::Misc_ShrinkOutburst);
				std::string message = std::format("Shrink Outburst 冷却中：{:.1f} 秒", cooldown);
				shake_camera(player, 0.75f, 0.35f);
				NotifyWithSound(player, message);
			}
			return;
		}
		ApplyActionCooldown(player, CooldownSource::Misc_ShrinkOutburst);
		DamageAV(player, ActorValue::kHealth, damagehp);
		ShrinkOutburstExplosion(player, false);
	}

	void ProtectSmallOnesEvent(const ManagedInputEvent& data) {
		static Timer ProtectTimer = Timer(5.0);
		if (ProtectTimer.ShouldRunFrame()) {
			Utils_ProtectTinies(SizeManager::BalancedMode());
		}
	}

	void RecallShrunkenActorsEvent(const ManagedInputEvent& data) {
		RecallShrunkenActors(true, true);
	}

	void ToggleSizeLimitRulesEvent(const ManagedInputEvent& data) {
		Config::Balance.bSizeLimitRulesEnabled ^= true;
		Notify("体型上限规则：{}", Config::Balance.bSizeLimitRulesEnabled ? "已启用" : "已禁用");

		for (auto actor : find_actors()) {
			if (!actor) {
				continue;
			}
			UpdateMaxScale(actor);
		}

		if (!Config::SaveSettings()) {
			logger::warn("Failed to save settings after ToggleSizeLimitRules");
		}
	}

	void AnimSpeedUpEvent(const ManagedInputEvent& data) {
		AnimationManager::AdjustAnimSpeed(0.045f); // Increase speed and power
	}

	void AnimSpeedDownEvent(const ManagedInputEvent& data) {
		AnimationManager::AdjustAnimSpeed(-0.045f); // Decrease speed and power
	}

	void AnimMaxSpeedEvent(const ManagedInputEvent& data) {
		AnimationManager::AdjustAnimSpeed(0.090f); // Strongest attack speed buff
	}

	void VoreInputEvent(const ManagedInputEvent& data) {
		static Timer voreTimer = Timer(0.25);
		auto pred = PlayerCharacter::GetSingleton();
		if (!pred || AnimationVars::General::IsGTSBusy(pred)) {
			return;
		}

		if (voreTimer.ShouldRunFrame()) {
			auto& VoreManager = VoreController::GetSingleton();

			std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, 1);
			for (auto prey: preys) {
				VoreManager.StartVore(pred, prey);
			}
		}
	}

	void VoreInputEvent_Follower(const ManagedInputEvent& data) {
		Actor* player = PlayerCharacter::GetSingleton();
		if (player) {
			ForceFollowerAnimation(player, FollowerAnimType::Vore);
		}
	}

	void StruggleInputEvent(const ManagedInputEvent& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto transient = Transient::GetActorData(player);
			if (transient) {
				float& EscapeProgress = transient->EscapingActionProgress;
				if (EscapeProgress < 1.0f) {
					float stamina = GetAV(player, ActorValue::kStamina);
					float stamina_req = 10.0f * EscapeProgress;

					if (stamina >= stamina_req) {
						DamageAV(player, ActorValue::kStamina, stamina_req);
						shake_camera(player, 2.75f * EscapeProgress, 0.35f);

						EscapeProgress += 1.0f / StruggleMax; // Need to struggle 40 times
						EscapeProgress = std::clamp(EscapeProgress, 0.0f, 1.0f); // Can't go higher than 1
						logger::info("Escape Progress: {}", EscapeProgress);
						if (EscapeProgress >= 1.0f) {
							transient->EscapingInteraction = true;
							ResetEscapeDataTask();
						}
					} else {
						std::string message = "体力不足，无法挣脱";
						shake_camera(player, 0.45f, 0.30f);
						NotifyWithSound(player, message);
					}
				}
			}
		}
	}

	


	//True for player false for fol;
	void ToggleCrawlImpl(const bool a_IsPlayer) {

		if (a_IsPlayer) {
			/// XOR Bit flip to toggle
			Persistent::EnableCrawlPlayer.value ^= true;
			const std::string Msg = fmt::format("玩家爬行：{}", Persistent::EnableCrawlPlayer.value ? "已启用" : "已禁用");
			RE::DebugNotification(Msg.c_str(),nullptr,false);
		}
		else {
			/// XOR Bit flip to toggle
			Persistent::EnableCrawlFollower.value ^= true;
			const std::string Msg = fmt::format("追随者爬行：{}", Persistent::EnableCrawlFollower.value ? "已启用" : "已禁用");
			RE::DebugNotification(Msg.c_str(),nullptr, false);
		}


	}

	void ToggleCrawlImpl_Player(const ManagedInputEvent& data) {
		ToggleCrawlImpl(true);
	}

	void ToggleCrawlImpl_Follower(const ManagedInputEvent& data) {
		ToggleCrawlImpl(false);
	}

	void PrintQuickStats(Actor* a_Actor) {

		if (!a_Actor) return;

		const bool Mammoth = Config::UI.sDisplayUnits == "kMammoth";
		float HH = HighHeelManager::GetBaseHHOffset(a_Actor)[2] / 100;
		const std::string HHOffset = (HighHeelManager::IsWearingHH(a_Actor) && !Mammoth) ? fmt::format(" + {}", GetFormatedHeight(HH)) : "";

		Notify("{}: ({:.2f}x) {}{}",
			a_Actor->GetName(),
			get_visual_scale(a_Actor),
			GetFormatedHeight(a_Actor),
			HHOffset
		);
	}

	void ShowQuickStats(const ManagedInputEvent& data) {
		auto Player = PlayerCharacter::GetSingleton();
		PrintQuickStats(Player);

		for (auto TeamMate : FindTeammates()) {
			PrintQuickStats(TeamMate);
		}
	}

	void OpenSkillTree(const ManagedInputEvent& data) {

		auto UI = UI::GetSingleton();
		if (!UI) return;

		if (State::IsInBlockingMenu() || UI->IsMenuOpen(DialogueMenu::MENU_NAME) || UI->IsMenuOpen(Console::MENU_NAME)) {
			Notify("打开技能树前，请先关闭其他菜单");
			return;
		}

		Runtime::SetFloat(Runtime::GLOB.GTSSkillMenu, 1.0);
	}

}

namespace GTS {

	void InputFunctions::Update() {
		if (!Config::Balance.bShrinkRecallAuto && !Config::Balance.bShrinkRecallNotifyNearby) {
			return;
		}

		if (!RecallShrunkenActorsCondition()) {
			return;
		}

		static Timer autoRecallTimer = Timer(0.0);
		static Timer nearbyNotifyTimer = Timer(0.0);

		if (Config::Balance.bShrinkRecallAuto) {
			autoRecallTimer.UpdateDelta(GetRecallAutoInterval());
			if (autoRecallTimer.ShouldRunFrame()) {
				RecallShrunkenActors(false, false);
			}
		}

		if (Config::Balance.bShrinkRecallNotifyNearby) {
			nearbyNotifyTimer.UpdateDelta(GetRecallNotifyInterval());
			if (nearbyNotifyTimer.ShouldRunFrame()) {
				NotifyNearbyShrunkenActors();
			}
		}
	}

	void InputFunctions::RegisterEvents() {

		InputManager::RegisterInputEvent("SizeReserve", SizeReserveEvent, SizeReserveCondition);
		InputManager::RegisterInputEvent("DisplaySizeReserve", DisplaySizeReserveEvent, SizeReserveCondition);
		InputManager::RegisterInputEvent("DebugReport", DebugReportEvent);
		InputManager::RegisterInputEvent("AnimSpeedUp", AnimSpeedUpEvent);
		InputManager::RegisterInputEvent("AnimSpeedDown", AnimSpeedDownEvent);
		InputManager::RegisterInputEvent("AnimMaxSpeed", AnimMaxSpeedEvent);
		InputManager::RegisterInputEvent("RapidGrowth", RapidGrowthEvent, RappidGrowShrinkCondition);
		InputManager::RegisterInputEvent("RapidShrink", RapidShrinkEvent, RappidGrowShrinkCondition);
		InputManager::RegisterInputEvent("ShrinkOutburst", ShrinkOutburstEvent, ShrinkOutburstCondition);
		InputManager::RegisterInputEvent("ProtectSmallOnes", ProtectSmallOnesEvent, ProtectSmallOnesCondition);
		InputManager::RegisterInputEvent("RecallShrunkenActors", RecallShrunkenActorsEvent, RecallShrunkenActorsCondition);
		InputManager::RegisterInputEvent("ToggleSizeLimitRules", ToggleSizeLimitRulesEvent, ToggleSizeLimitRulesCondition);

		InputManager::RegisterInputEvent("ManualGrow", TotalControlGrowEvent, TotalControlCondition);
		InputManager::RegisterInputEvent("ManualShrink", TotalControlShrinkEvent, TotalControlCondition);
		InputManager::RegisterInputEvent("ManualGrowOther", TotalControlGrowOtherEvent, TotalControlCondition);
		InputManager::RegisterInputEvent("ManualShrinkOther", TotalControlShrinkOtherEvent, TotalControlCondition);

		InputManager::RegisterInputEvent("ManualGrowOverTime", TotalControlGrowPlayer_OverTime, TotalControlCondition);
		InputManager::RegisterInputEvent("ManualShrinkOverTime", TotalControlShrinkPlayer_OverTime, TotalControlCondition);
		InputManager::RegisterInputEvent("ManualGrowOtherOverTime", TotalControlEnlargeTeammate_OverTime, TotalControlCondition);
		InputManager::RegisterInputEvent("ManualShrinkOtherOverTime", TotalControlShrinkTeammate_OverTime, TotalControlCondition);

		InputManager::RegisterInputEvent("Vore", VoreInputEvent, VoreCondition);
		InputManager::RegisterInputEvent("PlayerVore", VoreInputEvent_Follower, VoreCondition_Follower);

		InputManager::RegisterInputEvent("StruggleUp", StruggleInputEvent, StruggleCondition);
		InputManager::RegisterInputEvent("StruggleDown", StruggleInputEvent, StruggleCondition);
		InputManager::RegisterInputEvent("StruggleLeft", StruggleInputEvent, StruggleCondition);
		InputManager::RegisterInputEvent("StruggleRight", StruggleInputEvent, StruggleCondition);

		//Ported from papyrus
		InputManager::RegisterInputEvent("TogglePlayerCrawl", ToggleCrawlImpl_Player);
		InputManager::RegisterInputEvent("ToggleFollowerCrawl", ToggleCrawlImpl_Follower);
		InputManager::RegisterInputEvent("ShowQuickStats", ShowQuickStats);
		InputManager::RegisterInputEvent("OpenSkillTree", OpenSkillTree);
	}
}
