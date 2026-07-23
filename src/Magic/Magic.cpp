#include "Magic/Magic.hpp"

#include "Magic/Effects/Shouts/TinyCalamity.hpp"

#include "Magic/Effects/Enchantments/EnchGigantism.hpp"
#include "Magic/Effects/Enchantments/SwordOfSize.hpp"

#include "Magic/Effects/Spells/Absorb.hpp"
#include "Magic/Effects/Spells/GrowOther.hpp"
#include "Magic/Effects/Spells/Growth.hpp"
#include "Magic/Effects/Spells/GrowthSpurt.hpp"
#include "Magic/Effects/Spells/RestoreSize.hpp"
#include "Magic/Effects/Spells/RestoreSizeOther.hpp"
#include "Magic/Effects/Spells/Shrink.hpp"
#include "Magic/Effects/Spells/ShrinkFoe.hpp"
#include "Magic/Effects/Spells/ShrinkOther.hpp"
#include "Magic/Effects/Spells/SlowGrow.hpp"

#include "Magic/Effects/Potions/EssencePotion.hpp"
#include "Magic/Effects/Potions/ExperiencePotion.hpp"
#include "Magic/Effects/Potions/GrowthPotion.hpp"
#include "Magic/Effects/Potions/MaxSizePotion.hpp"
#include "Magic/Effects/Potions/MightPotion.hpp"
#include "Magic/Effects/Potions/ShrinkPotion.hpp"
#include "Magic/Effects/Potions/ShrinkResistPotion.hpp"
#include "Magic/Effects/Potions/SizeHunger.hpp"

#include "Magic/Effects/Poisons/PoisonOfShrinking.hpp"

namespace GTS {

	void Magic::OnStart() {}
	void Magic::OnUpdate() {}
	void Magic::OnFinish() {}

	std::string Magic::GetName() {
		return "";
	}

	Magic::Magic(ActiveEffect* effect) : activeEffect(effect) {

		if (this->activeEffect) {
			auto spell = this->activeEffect->spell;
			this->effectSetting = this->activeEffect->GetBaseObject();
			if (MagicTarget* m_target = this->activeEffect->target) {
				if (m_target->MagicTargetIsActor()) {
					this->target = skyrim_cast<Actor*>(m_target);
				}
			}
			if (this->activeEffect->caster) {
				this->caster = this->activeEffect->caster.get().get();
			}
			this->hasDuration = this->HasDuration();
		}
	}

	bool Magic::IsFinished() const {
		return this->state == State::CleanUp;
	}

	bool Magic::HasDuration() const {

		if (this->activeEffect) {
			if (auto spell = this->activeEffect->spell) {
				switch (spell->GetCastingType()) {
					case  MagicSystem::CastingType::kConstantEffect: {
						return false;
					}
					default: break;
				}
			}
		}
		
		if (auto effectSetting = this->effectSetting) {
			if (effectSetting->data.flags.all(EffectSetting::EffectSettingData::Flag::kNoDuration)) {
				return false;
			}
		}

		return true;
	}

	void Magic::Poll() {

		switch (this->state) {
			case State::Init: {
				this->dual_casted = this->IsDualCasting();
				this->state = State::Start;
				break;
			}
			case State::Start: {
				this->OnStart();
				this->state = State::Update;
				break;
			}
			case State::Update: {
				if (this->activeEffect->flags & ActiveEffect::Flag::kInactive) {
					break;
				}
				this->OnUpdate();
				if ((this->activeEffect->flags & ActiveEffect::Flag::kDispelled)
				    || (this->hasDuration && (this->activeEffect->elapsedSeconds >= this->activeEffect->duration))) {
					this->state = State::Finish;
				}
				break;
			}
			case State::Finish: {
				this->OnFinish();
				this->state = State::CleanUp;
				break;
			}
			case State::CleanUp: {
				break;
			}
		}
	}

	void Magic::Finish() {
		if (this->state == State::CleanUp) {
			return;
		}

		if (this->state == State::Update || this->state == State::Finish) {
			this->OnFinish();
		}
		this->state = State::CleanUp;
		this->activeEffect = nullptr;
	}

	Actor* Magic::GetTarget() const {
		return this->target;
	}

	Actor* Magic::GetCaster() const {
		return this->caster;
	}

	ActiveEffect* Magic::GetActiveEffect() const {
		return this->activeEffect;
	}

	EffectSetting* Magic::GetBaseEffect() const {
		return this->effectSetting;
	}

	void Magic::Dispel() const {
		if (this->activeEffect) {
			this->activeEffect->Dispel(false); // Not forced
			// Seems to be CTD prone for some reason, best to not use it
		}
	}

	bool Magic::IsDualCasting() const {
		if (this->caster && this->activeEffect) {
			auto casting_type = this->activeEffect->castingSource;
			if (casting_type == MagicSystem::CastingSource::kLeftHand || casting_type == MagicSystem::CastingSource::kRightHand) {
				if (auto source = this->caster->GetMagicCaster(casting_type)) {
					return source->GetIsDualCasting();
				}
			}
		}
		return false;
	}

	bool Magic::DualCasted() const {
		return this->dual_casted;
	}

	void MagicManager::ProcessActiveEffects(Actor* a_actor) {
		if (!a_actor) {
			return;
		}

		auto* magicTarget = a_actor->AsMagicTarget();
		if (!magicTarget) {
			MagicManager::GetSingleton().ResetActor(a_actor);
			return;
		}

		BSSimpleList<ActiveEffect*>* effect_list = magicTarget->GetActiveEffectList();

		if (!effect_list) {
			MagicManager::GetSingleton().ResetActor(a_actor);
			return;
		}

		std::scoped_lock lock(activeEffectsLock);
		std::unordered_set<ActiveEffect*> currentEffects;

		for (ActiveEffect* effect : (*effect_list)) {
			if (!effect) {
				continue;
			}
			currentEffects.insert(effect);

			if (!active_effects.contains(effect)) {
				EffectSetting* base_spell = effect->GetBaseObject();
				auto factorySearch = factories.find(base_spell);
				if (factorySearch != factories.end()) {
					auto& factory = factorySearch->second;
					auto magic_effect = factory->MakeNew(effect);
					if (magic_effect) {
						active_effects.try_emplace(effect, TrackedMagic{ std::move(magic_effect), a_actor });
					}
				}
			}

			if (auto it = active_effects.find(effect); it != active_effects.end()) {
				it->second.effect->Poll();
				if (it->second.effect->IsFinished()) {
					active_effects.erase(it);
				}
			}
		}

		for (auto it = active_effects.begin(); it != active_effects.end();) {
			if (it->second.owner == a_actor && !currentEffects.contains(it->first)) {
				it->second.effect->Finish();
				it = active_effects.erase(it);
			}
			else {
				++it;
			}
		}
	}

	std::string MagicManager::DebugName() {
		return "::MagicManager";
	}

	void MagicManager::ActorUpdate(RE::Actor* actor) {
		ProcessActiveEffects(actor);
	}

	void MagicManager::Update() {
		// Active effects are polled while their owning actor's effect list is valid.
	}

	void MagicManager::Reset() {
		std::scoped_lock lock(activeEffectsLock);
		for (auto& tracked : active_effects | std::views::values) {
			tracked.effect->Finish();
		}
		this->active_effects.clear();
	}

	void MagicManager::ResetActor(RE::Actor* actor) {
		if (!actor) {
			return;
		}

		std::scoped_lock lock(activeEffectsLock);
		for (auto it = active_effects.begin(); it != active_effects.end();) {
			if (it->second.owner == actor) {
				it->second.effect->Finish();
				it = active_effects.erase(it);
			}
			else {
				++it;
			}
		}
	}

	void MagicManager::DataReady() {

		// -------- Potions
		RegisterMagic<MaxSizePotion>(Runtime::MGEF.GTSPotionEffectSizeLimitWeak);
		RegisterMagic<MaxSizePotion>(Runtime::MGEF.GTSPotionEffectSizeLimitNormal);
		RegisterMagic<MaxSizePotion>(Runtime::MGEF.GTSPotionEffectSizeLimitStrong);
		RegisterMagic<MaxSizePotion>(Runtime::MGEF.GTSPotionEffectSizeLimitExtreme);
		RegisterMagic<MaxSizePotion>(Runtime::MGEF.GTSAlchEffectSizeLimit);

		RegisterMagic<MightPotion>(Runtime::MGEF.GTSPotionEffectMightWeak);
		RegisterMagic<MightPotion>(Runtime::MGEF.GTSPotionEffectMightNormal);
		RegisterMagic<MightPotion>(Runtime::MGEF.GTSPotionEffectMightStrong);
		RegisterMagic<MightPotion>(Runtime::MGEF.GTSPotionEffectMightExtreme);
		RegisterMagic<MightPotion>(Runtime::MGEF.GTSAlchEffectMight);

		RegisterMagic<EssencePotion>(Runtime::MGEF.GTSPotionEffectEssenceWeak);
		RegisterMagic<EssencePotion>(Runtime::MGEF.GTSPotionEffectEssenceNormal);
		RegisterMagic<EssencePotion>(Runtime::MGEF.GTSPotionEffectEssenceStrong);
		RegisterMagic<EssencePotion>(Runtime::MGEF.GTSPotionEffectEssenceExtreme);
		RegisterMagic<EssencePotion>(Runtime::MGEF.GTSAlchEffectEssence);

		RegisterMagic<ShrinkResistPotion>(Runtime::MGEF.GTSPotionEffectResistShrinkWeak);
		RegisterMagic<ShrinkResistPotion>(Runtime::MGEF.GTSPotionEffectResistShrinkNormal);
		RegisterMagic<ShrinkResistPotion>(Runtime::MGEF.GTSPotionEffectResistShrinkStrong);
		RegisterMagic<ShrinkResistPotion>(Runtime::MGEF.GTSPotionEffectResistShrinkExtreme);
		RegisterMagic<ShrinkResistPotion>(Runtime::MGEF.GTSAlchEffectResistShrink);

		RegisterMagic<GrowthPotion>(Runtime::MGEF.GTSPotionEffectGrowthWeak);
		RegisterMagic<GrowthPotion>(Runtime::MGEF.GTSPotionEffectGrowthNormal);
		RegisterMagic<GrowthPotion>(Runtime::MGEF.GTSPotionEffectGrowthStrong);
		RegisterMagic<GrowthPotion>(Runtime::MGEF.GTSPotionEffectGrowthExtreme);
		RegisterMagic<GrowthPotion>(Runtime::MGEF.GTSAlchEffectGrowth);

		RegisterMagic<ShrinkPotion>(Runtime::MGEF.GTSPotionEffectSizeDrain);
		RegisterMagic<Shrink_Poison>(Runtime::MGEF.GTSPoisonEffectShrinking);

		RegisterMagic<ExperiencePotion>(Runtime::MGEF.GTSPotionEffectSizeExperienceBasic);
		RegisterMagic<SizeHunger>(Runtime::MGEF.GTSPotionEffectSizeHunger);

		// -------- Enchantments
		RegisterMagic<Gigantism>(Runtime::MGEF.GTSEnchGigantism);
		RegisterMagic<SwordOfSize>(Runtime::MGEF.GTSEnchSwordAbsorbSize);

		// -------- Shouts
		RegisterMagic<TinyCalamity>(Runtime::MGEF.GTSEffectTinyCalamity);

		RegisterMagic<GrowthSpurt>(Runtime::MGEF.GTSEffectGrowthSpurt1);
		RegisterMagic<GrowthSpurt>(Runtime::MGEF.GTSEffectGrowthSpurt2);
		RegisterMagic<GrowthSpurt>(Runtime::MGEF.GTSEffectGrowthSpurt3);

		RegisterMagic<Absorb>(Runtime::MGEF.GTSEffectAbsorb);
		RegisterMagic<Absorb>(Runtime::MGEF.GTSEffectAbsorbTrue);

		// -------- Spells
		RegisterMagic<ShrinkFoe>(Runtime::MGEF.GTSEffectShrinkEnemy);
		RegisterMagic<ShrinkFoe>(Runtime::MGEF.GTSEffectShrinkEnemyAOE);
		RegisterMagic<ShrinkFoe>(Runtime::MGEF.GTSEffectShrinkOtherAOEMastery);
		RegisterMagic<ShrinkFoe>(Runtime::MGEF.GTSEffectShrinkBolt);
		RegisterMagic<ShrinkFoe>(Runtime::MGEF.GTSEffectShrinkStorm);

		RegisterMagic<SlowGrow>(Runtime::MGEF.GTSEffectSlowGrowth);
		RegisterMagic<SlowGrow>(Runtime::MGEF.GTSEffectSlowGrowthDual);
	
		RegisterMagic<Growth>(Runtime::MGEF.GTSEffectGrowth);
		RegisterMagic<Growth>(Runtime::MGEF.GTSEffectGrowthAdept);
		RegisterMagic<Growth>(Runtime::MGEF.GTSEffectGrowthExpert);

		RegisterMagic<GrowOther>(Runtime::MGEF.GTSEffectGrowAlly);
		RegisterMagic<GrowOther>(Runtime::MGEF.GTSEffectGrowAllyAdept);
		RegisterMagic<GrowOther>(Runtime::MGEF.GTSEffectGrowAllyExpert);

		RegisterMagic<ShrinkOther>(Runtime::MGEF.GTSEffectShrinkAlly);
		RegisterMagic<ShrinkOther>(Runtime::MGEF.GTSEffectShrinkAllyAdept);
		RegisterMagic<ShrinkOther>(Runtime::MGEF.GTSEffectShrinkAllyExpert);

		RegisterMagic<RestoreSize>(Runtime::MGEF.GTSEffectRestoreSize);
		RegisterMagic<RestoreSizeOther>(Runtime::MGEF.GTSEffectRestoreSizeOther);

		RegisterMagic<Shrink>(Runtime::MGEF.GTSEffectShrink);
	}
}
