#include "Utils/Actor/ActorUtils.hpp"

#include "Config/Config.hpp"
#include "Utils/Actor/ActorBools.hpp"

#include "Managers/AI/AIFunctions.hpp"
#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Managers/Damage/LaunchActor.hpp"

#include "Systems/Colliders/ActorCollisionData.hpp"
#include "Systems/Misc/Time.hpp"
#include "Utils/Actor/FindActor.hpp"
#include <limits>
#include <unordered_map>

/* Actor Utils
 * Contains general helper functions
 * related to manipulating skyrim actors.
 */

namespace {
	
	const std::vector<const char*> disarm_nodes = {
	"WEAPON",
	"SHIELD",
	"NPC L MagicNode [LMag]",
	"NPC R MagicNode [RMag]",
	/*"WeaponDagger",
	"WeaponAxe",
	"WeaponSword",
	"WeaponMace",
	"WEAPON",
	"SHIELD",
	"QUIVER",
	"WeaponBow",
	"WeaponBack",
	"AnimObjectL",
	"AnimObjectR",
	"NPC L MagicNode [LMag]",
	"NPC R MagicNode [RMag]",
	*/
	};

	struct CharacterControllerCache {
		std::uint64_t frame = std::numeric_limits<std::uint64_t>::max();
		std::unordered_map<RE::bhkCharacterController*, RE::Actor*> actorByController;
	};

	CharacterControllerCache& GetCharacterControllerCache() {
		thread_local CharacterControllerCache cache;
		const auto currentFrame = GTS::Time::FramesElapsed();
		if (cache.frame == currentFrame) {
			return cache;
		}

		cache.frame = currentFrame;
		cache.actorByController.clear();

		for (auto actor : GTS::find_actors()) {
			if (!actor) {
				continue;
			}
			if (auto* controller = actor->GetCharController()) {
				cache.actorByController.try_emplace(controller, actor);
			}
		}

		return cache;
	}
}

namespace GTS {

	Actor* GetActorPtr(Actor* a_actor) {
		return a_actor;
	}

	Actor* GetActorPtr(Actor& a_actor) {
		return &a_actor;
	}

	Actor* GetActorPtr(ActorHandle& a_actor) {
		if (!a_actor) {
			return nullptr;
		}
		return a_actor.get().get();
	}

	Actor* GetActorPtr(const ActorHandle& a_actor) {
		if (!a_actor) {
			return nullptr;
		}
		return a_actor.get().get();
	}

	Actor* GetActorPtr(FormID a_formID) {
		Actor* actor = TESForm::LookupByID<Actor>(a_formID);
		if (!actor) {
			return nullptr;
		}
		return actor;
	}

	Actor* GetCharContActor(bhkCharacterController* a_charController) {
		if (!a_charController) {
			return nullptr;
		}

		auto& cache = GetCharacterControllerCache();
		if (auto it = cache.actorByController.find(a_charController); it != cache.actorByController.end()) {
			return it->second;
		}
		return nullptr;
	}

	RE::NiPoint3 RotateAngleAxis(const RE::NiPoint3& a_vec, const float a_angle, const RE::NiPoint3& a_axis) {
		float S = sin(a_angle);
		float C = cos(a_angle);

		const float XX = a_axis.x * a_axis.x;
		const float YY = a_axis.y * a_axis.y;
		const float ZZ = a_axis.z * a_axis.z;

		const float XY = a_axis.x * a_axis.y;
		const float YZ = a_axis.y * a_axis.z;
		const float ZX = a_axis.z * a_axis.x;

		const float XS = a_axis.x * S;
		const float YS = a_axis.y * S;
		const float ZS = a_axis.z * S;

		const float OMC = 1.f - C;

		return {
			(OMC * XX + C) * a_vec.x + (OMC * XY - ZS) * a_vec.y + (OMC * ZX + YS) * a_vec.z,
			(OMC * XY + ZS) * a_vec.x + (OMC * YY + C) * a_vec.y + (OMC * YZ - XS) * a_vec.z,
			(OMC * ZX - YS) * a_vec.x + (OMC * YZ + XS) * a_vec.y + (OMC * ZZ + C) * a_vec.z
		};
	}

	void SetSneaking(Actor* a_target, bool a_doOverrideSneak, int a_sneakState) {
		if (a_doOverrideSneak) {
			a_target->AsActorState()->actorState1.sneaking = a_sneakState;
			return;
		}
		
		if (TransientActorData* transient = Transient::GetActorData(a_target)) {
			a_target->AsActorState()->actorState1.sneaking = transient->WasSneaking;
		}
		
	}

	void SetReanimatedState(Actor* a_target) {
		
		// disallow to override it again if it returned true a frame before
		if (!WasReanimated(a_target)) { 
			bool reanimated = a_target->AsActorState()->GetLifeState() == ACTOR_LIFE_STATE::kReanimate;
			if (auto transient = Transient::GetActorData(a_target)) {
				transient->WasReanimated = reanimated;
			}
		}
	}

	//Disables idle greetings by not allowing the timer to expire
	void ShutUp(Actor* a_target) {
		if (!a_target) {
			return;
		}
		if (!a_target->IsPlayerRef()) {
			if (AIProcess* ai = a_target->GetActorRuntimeData().currentProcess) {
				if (ai->high) {
					ai->high->greetingTimer = 5;
				}
			}
		}
	}

	void DecreaseShoutCooldown(Actor* a_target) {
		AIProcess* process = a_target->GetActorRuntimeData().currentProcess;
		if (process && TinyCalamityHasRefresh(a_target)) {
			HighProcessData* high = process->high;
			float by = 0.90f;
			if (high) {
				float& rec_time = high->voiceRecoveryTime;
				//log::info("Recovery Time: {}", rec_time);
				if (rec_time > 0.0f) {
					TinyCalamityActive(a_target) ? by = 0.85f : by = 0.90f;

					rec_time < 0.0f ? rec_time == 0.0f : rec_time *= by;

					//log::info("New Recovery Time: {}", rec_time);
				}
			}
		}
	}

	void Disintegrate(Actor* a_target) {
		if (!a_target) {
			return;
		}

		const std::string taskname = std::format("Disintegrate_{}", a_target->formID);
		ActorHandle targetRef = a_target->CreateRefHandle();
		TaskManager::RunOnce(taskname, [=](auto&) {
			if (!targetRef) {
				return;
			}
			Actor* tiny = targetRef.get().get();
			if (!tiny) {
				return;
			}
			tiny->SetCriticalStage(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
		});
	}

	void SetRestrained(Actor* a_actor) {
		CallVMFunctionOn(a_actor, "Actor", "SetRestrained", true);
	}

	void SetUnRestrained(Actor* a_actor) {
		CallVMFunctionOn(a_actor, "Actor", "SetRestrained", false);
	}

	void SetDontMove(Actor* a_actor) {
		CallVMFunctionOn(a_actor, "Actor", "SetDontMove", true);
	}

	void SetMove(Actor* a_actor) {
		CallVMFunctionOn(a_actor, "Actor", "SetDontMove", false);
	}

	void KnockAreaEffect(TESObjectREFR* a_sourceRef, float a_magnitude, float a_radius) {
		CallVMFunctionOn(a_sourceRef, "ObjectReference", "KnockAreaEffect", a_magnitude, a_radius);
	}

	void ForceRagdoll(Actor* a_target, bool a_enableRagDoll) {
		if (!a_target) {
			return;
		}
		auto charCont = a_target->GetCharController();
		if (!charCont) {
			return;
		}
		BSAnimationGraphManagerPtr animGraphManager;
		if (a_target->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					if (graph->HasRagdoll()) {
						if (a_enableRagDoll) {
							graph->AddRagdollToWorld();
							charCont->flags.set(CHARACTER_FLAGS::kFollowRagdoll);
						}
						else {
							graph->RemoveRagdollFromWorld();
							charCont->flags.reset(CHARACTER_FLAGS::kFollowRagdoll);
						}
					}
				}
			}
		}
	}

	std::vector<hkpRigidBody*> GetActorRB(Actor* a_actor) {
		std::vector<hkpRigidBody*> results = {};
		auto charCont = a_actor->GetCharController();
		if (!charCont) {
			return results;
		}

		bhkCharProxyController* charProxyController = skyrim_cast<bhkCharProxyController*>(charCont);
		bhkCharRigidBodyController* charRigidBodyController = skyrim_cast<bhkCharRigidBodyController*>(charCont);
		if (charProxyController) {
			// Player controller is a proxy one
			auto& proxy = charProxyController->proxy;
			hkReferencedObject* refObject = proxy.referencedObject.get();
			if (refObject) {
				hkpCharacterProxy* hkpObject = skyrim_cast<hkpCharacterProxy*>(refObject);

				if (hkpObject) {
					// Not sure what bodies is doing
					for (auto body : hkpObject->bodies) {
						results.push_back(body);
					}
					// // This one appears to be active during combat.
					// // Maybe used for sword swing collision detection
					// for (auto phantom: hkpObject->phantoms) {
					// 	results.push_back(phantom);
					// }
					//
					// // This is the actual shape
					// if (hkpObject->shapePhantom) {
					// 	results.push_back(hkpObject->shapePhantom);
					// }
				}
			}
		}
		else if (charRigidBodyController) {
			// NPCs seem to use rigid body ones
			auto& characterRigidBody = charRigidBodyController->characterRigidBody;
			hkReferencedObject* refObject = characterRigidBody.referencedObject.get();
			if (refObject) {
				hkpCharacterRigidBody* hkpObject = skyrim_cast<hkpCharacterRigidBody*>(refObject);
				if (hkpObject) {
					if (hkpObject->m_character) {
						results.push_back(hkpObject->m_character);
					}
				}
			}
		}

		return results;
	}

	void PushActorAway(Actor* a_source, Actor* a_receiver, float a_force) {
		if (!a_source || !a_receiver || a_source == a_receiver) {
			return;
		}
		if (a_receiver->IsDead() || AnimationVars::Tiny::IsBeingShrunk(a_receiver)) {
			return;
		}

		// Force < 1.0 can introduce weird sliding issues to Actors, not recommended to pass force < 1.0.
		a_force = std::max(1.0f, a_force);

		AIProcess* ai = a_source->GetActorRuntimeData().currentProcess;
		if (!ai || !a_receiver->Is3DLoaded() || !a_source->Is3DLoaded()) {
			return;
		}

		NiPoint3 direction = a_receiver->GetPosition() - a_source->GetPosition();
		const float directionLength = direction.Length();
		if (directionLength <= 1e-4f) {
			return;
		}
		direction = direction / directionLength;
		typedef void (*DefPushActorAway)(AIProcess* ai, Actor* actor, NiPoint3& direction, float force);
		REL::Relocation<DefPushActorAway> RealPushActorAway{ REL::RelocationID(38858, 39895, NULL) };
		RealPushActorAway(ai, a_receiver, direction, a_force);
	}

	void ApplyManualHavokImpulse(Actor* a_target, float a_forceX, float a_forceY, float a_forceZ, float a_multiplier) {
		// For this function to work, actor must be pushed away first. 
		// It may be a good idea to wait about 0.05 sec before callind it after actor has been pushed, else it may not work
		if (!a_target || !a_target->Is3DLoaded()) {
			return;
		}
		auto model = a_target->Get3D(false);
		if (!model) {
			return;
		}

		hkVector4 impulse = hkVector4(a_forceX * a_multiplier, a_forceY * a_multiplier, a_forceZ * a_multiplier, 1.0f);

		if (bhkCollisionObject* collision = model->GetCollisionObject()) {
			if (auto rigidbody = collision->GetRigidBody()) {
				if (auto body = rigidbody->AsBhkRigidBody()) {
					body->SetLinearImpulse(impulse);
					//log::info("Bdy found, Applying impulse {} to {}", Vector2Str(impulse), a_target->GetDisplayFullName());
				}
			}
		}
	}

	void DisableCollisions(Actor* a_actor, TESObjectREFR* a_target) {
		if (a_actor) {
			if (TransientActorData* data = Transient::GetActorData(a_actor)) {
				data->DisableCollisionWith.store(a_target ? a_target->formID : 0, std::memory_order_release);
				ActorCollisionData colliders = ActorCollisionData(a_actor);
				colliders.UpdateCollisionFilter();
				if (a_target) {
					if (Actor* asOtherActor = skyrim_cast<Actor*>(a_target)) {
						ActorCollisionData otherColliders = ActorCollisionData(asOtherActor);
						otherColliders.UpdateCollisionFilter();
					}
				}
			}
		}
	}

	void EnableCollisions(Actor* a_actor) {
		if (a_actor) {
			if (TransientActorData* data = Transient::GetActorData(a_actor)) {
				const FormID otherActorFormID = data->DisableCollisionWith.exchange(0, std::memory_order_acq_rel);
				ActorCollisionData colliders = ActorCollisionData(a_actor);
				colliders.UpdateCollisionFilter();
				if (otherActorFormID != 0) {
					if (auto otherActor = TESForm::LookupByID<Actor>(otherActorFormID)) {
						auto otherColliders = ActorCollisionData(otherActor);
						otherColliders.UpdateCollisionFilter();
					}
				}
			}
		}
	}


	hkaRagdollInstance* GetRagdoll(Actor* a_actor) {
		if (!a_actor) {
			return nullptr;
		}

		BSAnimationGraphManagerPtr animGraphManager;
		if (a_actor->GetAnimationGraphManager(animGraphManager)) {
			for (BSTSmartPointer<BShkbAnimationGraph>& graph : animGraphManager->graphs) {
				if (graph) {
					hkbCharacter& character = graph->characterInstance;
					if (hkbRagdollDriver* ragdollDriver = character.ragdollDriver.get()) {
						if (hkaRagdollInstance* ragdoll = ragdollDriver->ragdoll) {
							return ragdoll;
						}
					}
				}
			}
		}
		return nullptr;
	}

	void DisarmActor(Actor* a_target, bool a_dropWeapon) {
		if (!a_target) {
			return;
		}

		if (!a_dropWeapon) {
			for (const char* const& node : disarm_nodes) {
				if (auto object = find_node(a_target, node)) {
					object->local.scale = 0.01f;
					update_node(object);

					std::string objectname = object->name.c_str();
					std::string name = std::format("ScaleWeapons_{}_{}", a_target->formID, objectname);
					std::string nodeName = objectname;
					ActorHandle tinyHandle = a_target->CreateRefHandle();

					double Start = Time::WorldTimeElapsed();

					TaskManager::Run(name, [tinyHandle, nodeName, Start](auto&) {
						if (!tinyHandle) {
							return false;
						}
						Actor* Tiny = tinyHandle.get().get();
						if (!Tiny) {
							return false;
						}
						auto currentObject = find_node(Tiny, nodeName);
						if (!currentObject) {
							return false;
						}
						double Finish = Time::WorldTimeElapsed();

						currentObject->local.scale = 0.01f;
						update_node(currentObject);

						if (Finish - Start > 0.25 && !AnimationVars::General::IsGTSBusy(Tiny)) {
							currentObject->local.scale = 1.0f;
							update_node(currentObject);
							return false;
						}

						return true;
					});
				}
			}
		}
		else {
			NiPoint3 dropPos = a_target->GetPosition();
			for (TESForm* object : { a_target->GetEquippedObject(true), a_target->GetEquippedObject(false) }) {
				dropPos.x += 25 * get_visual_scale(a_target);
				dropPos.y += 25 * get_visual_scale(a_target);
				if (object) {
					logger::debug("Object found");

					if (TESBoundObject* as_object = skyrim_cast<TESBoundObject*>(object)) {
						logger::debug("As object exists, {}", as_object->GetName());
						dropPos.z += 40 * get_visual_scale(a_target);
						a_target->RemoveItem(as_object, 1, ITEM_REMOVE_REASON::kDropping, nullptr, nullptr, &dropPos, nullptr);
					}
				}
			}
		}
	}

	void ManageRagdoll(Actor* a_actor, float a_deltaLength, NiPoint3 a_deltaLocation, NiPoint3 a_targetLocation) {
		if (!a_actor) {
			return;
		}

		if (a_deltaLength >= 70.0f) {
			// WARP if > 1m
			hkaRagdollInstance* ragDoll = GetRagdoll(a_actor);
			if (!ragDoll) {
				return;
			}
			hkVector4 delta = hkVector4(a_deltaLocation.x / 70.0f, a_deltaLocation.y / 70.0f, a_deltaLocation.z / 70, 1.0f);
			for (auto rb : ragDoll->rigidBodies) {
				if (rb) {
					auto ms = rb->GetMotionState();
					if (ms) {
						hkVector4 currentPos = ms->transform.translation;
						hkVector4 newPos = currentPos + delta;
						rb->motion.SetPosition(newPos);
						rb->motion.SetLinearVelocity(hkVector4(0.0f, 0.0f, -10.0f, 0.0f));
					}
				}
			}
		}
		else {
			// Just move the hand if <1m
			constexpr std::string_view handNodeName = "NPC HAND L [L Hand]";
			if (NiAVObject* handBone = find_node(a_actor, handNodeName)) {
				if (bhkCollisionObject* collisionHand = handBone->GetCollisionObject()) {
					if (bhkRigidBody* handRbBhk = collisionHand->GetRigidBody()) {
						if (hkpRigidBody* handRb = static_cast<hkpRigidBody*>(handRbBhk->referencedObject.get())) {
							hkVector4 targetLocationHavok = hkVector4(a_targetLocation.x / 70.0f, a_targetLocation.y / 70.0f, a_targetLocation.z / 70.0f, 1.0f);
							handRb->motion.SetPosition(targetLocationHavok);
							handRb->motion.SetLinearVelocity(hkVector4(0.0f, 0.0f, -10.0f, 0.0f));
						}
					}
				}
			}
		}
	}

	void ChanceToScare(Actor* a_giant, Actor* a_tiny, float a_duration, int a_random, bool a_useSizeDifference) {
		if (a_tiny->IsPlayerRef() || IsTeammate(a_tiny)) {
			return;
		}

		if (a_tiny->HasKeywordString("GTSKeyword_PlayerBFF") && a_giant->IsPlayerRef() || 
			a_tiny->HasKeywordString("GTSKeyword_FollowerBFF") && IsTeammate(a_giant)) {
			return;
		}

		float sizedifference = get_scale_difference(a_giant, a_tiny, SizeType::VisualScale, true, true);
		if (sizedifference > 1.15f && !a_tiny->IsDead()) {
			int rng = RandomInt(1, a_random);
			if (a_useSizeDifference) {
				rng = static_cast<int>(static_cast<float>(rng) / sizedifference);
			}
			if (rng <= 2.0f * sizedifference) {
				bool IsScared = IsActionOnCooldown(a_tiny, CooldownSource::Action_ScareOther);
				if (!IsScared && GetAV(a_tiny, ActorValue::kConfidence) > 0) {
					ApplyActionCooldown(a_tiny, CooldownSource::Action_ScareOther);
					ForceFlee(a_giant, a_tiny, a_duration, true); // always scare
				}
			}
		}
	}

	void StaggerActor(Actor* a_target, float a_power) {
		if (a_target->IsDead() || IsRagdolled(a_target) || GetAV(a_target, ActorValue::kHealth) <= 0.0f) {
			return;
		}
		AnimationVars::Other::SetStaggerMagnitude(a_target, a_power);
		a_target->NotifyAnimationGraph("staggerStart");
	}

	void StaggerActor(Actor* a_source, Actor* a_target, float a_power) {
		if (a_target->IsDead() || IsRagdolled(a_target) || AnimationVars::Tiny::IsBeingHugged(a_target) || GetAV(a_target, ActorValue::kHealth) <= 0.0f) {
			return;
		}
		if (!AnimationVars::Tiny::IsBeingShrunk(a_target)) {
			a_target->StaggerDirectional(a_source, a_power);
		}
	}

	void StaggerActor_Around(Actor* a_source, const float a_radius, bool a_doLaunch) {
		if (!a_source) {
			return;
		}
		const NiAVObject* node = find_node(a_source, "NPC Root [Root]");
		if (!node) {
			return;
		}
		NiPoint3 NodePosition = node->world.translate;

		const float giantScale = get_visual_scale(a_source) * GetSizeFromBoundingBox(a_source);
		const float CheckDistance = a_radius * giantScale;

		if (DebugDraw::CanDraw(a_source, DebugDraw::DrawTarget::kAnyGTS)) {
			DebugDraw::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), CheckDistance, 600, { 0.0f, 1.0f, 0.0f, 1.0f });
		}

		NiPoint3 giantLocation = a_source->GetPosition();
		for (Actor* otherActor : find_actors()) {
			if (otherActor != a_source) {
				NiPoint3 actorLocation = otherActor->GetPosition();
				if ((actorLocation - giantLocation).Length() < CheckDistance * 3) {
					int nodeCollisions = 0;

					auto model = otherActor->GetCurrent3D();

					if (model) {
						VisitNodes(model, [&nodeCollisions, NodePosition, CheckDistance](NiAVObject& a_obj) {
							float distance = (NodePosition - a_obj.world.translate).Length();
							if (distance < CheckDistance) {
								nodeCollisions += 1;
								return false;
							}
							return true;
							});
					}
					if (nodeCollisions > 0) {
						if (!a_doLaunch) {
							StaggerActor(a_source, otherActor, 0.50f);
						}
						else {
							if (get_scale_difference(a_source, otherActor, SizeType::VisualScale, true, false) < Push_Jump_Launch_Threshold) {
								StaggerActor(a_source, otherActor, 0.50f);
							}
							else {
								float launch_power = 0.33f;
								if (TinyCalamityActionBoostActive(a_source)) {
									launch_power *= 6.0f;
								}
								LaunchActor::ApplyLaunchTo(a_source, otherActor, 1.0f, launch_power);
							}
						}
					}
				}
			}
		}
	}

	void PushForward(Actor* a_source, Actor* a_target, float a_power) {
		if (!a_source || !a_target) {
			return;
		}

		double startTime = Time::WorldTimeElapsed();
		ActorHandle tinyHandle = a_target->CreateRefHandle();
		ActorHandle gianthandle = a_source->CreateRefHandle();
		std::string taskname = std::format("PushOther_{}", a_target->formID);
		PushActorAway(a_source, a_target, 1.0f);
		TaskManager::Run(taskname, [=](auto& update) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyHandle) {
				return false;
			}
			Actor* giant = gianthandle.get().get();
			Actor* tiny = tinyHandle.get().get();
			if (!giant || !tiny) {
				return false;
			}

			double endTime = Time::WorldTimeElapsed();

			if ((endTime - startTime) > 0.08) {
				auto model = giant->GetCurrent3D();
				if (!giant->Is3DLoaded() || !model) {
					return false;
				}

				auto playerRotation = model->world.rotate;
				RE::NiPoint3 localForwardVector{ 0.f, 1.f, 0.f };
				RE::NiPoint3 direction = playerRotation * localForwardVector;
				ApplyManualHavokImpulse(tiny, direction.x, direction.y, direction.z, a_power);
				return false;
			}
			return true;
		});
	}

	void PushTowards(Actor* a_source, Actor* a_target, std::string_view a_boneName, float a_power, bool a_doSizeCheck) {
		if (!a_source || !a_target) {
			return;
		}

		if (!AllowStagger(a_target)) {
			return;
		}
		NiAVObject* node = find_node(a_source, a_boneName);
		if (node) {
			PushTowards(a_source, a_target, node, a_power, a_doSizeCheck);
		}
	}

	void PushTowards_Task(const ActorHandle& a_sourceHandle, const ActorHandle& a_targetHandle, const NiPoint3& a_startCoords, const NiPoint3& a_endCoords, std::string_view a_taskName, float a_power, bool a_doSizeCheck) {

		const double startTime = Time::WorldTimeElapsed();

		TaskManager::RunFor(a_taskName, 2, [=](auto& update) {
			if (!a_sourceHandle) {
				return false;
			}
			if (!a_targetHandle) {
				return false;
			}
			Actor* giant = a_sourceHandle.get().get();
			Actor* tiny = a_targetHandle.get().get();

			double endTime = Time::WorldTimeElapsed();
			if (!giant || !tiny) {
				return false;
			}
			if (!tiny->Is3DLoaded()) {
				return true;
			}
			if (!tiny->GetCurrent3D()) {
				return true;
			}
			if ((endTime - startTime) > 0.05) {
				// Enough time has elapsed

				NiPoint3 vector = a_endCoords - a_startCoords;
				float distanceTravelled = vector.Length();
				if (distanceTravelled <= 1e-4f) {
					return false;
				}
				float timeTaken = static_cast<float>(endTime - startTime);
				float speed = (distanceTravelled / timeTaken) / GetAnimationSlowdown(giant);
				NiPoint3 direction = vector / distanceTravelled;

				if (a_doSizeCheck) {
					float giantscale = get_visual_scale(giant);
					float tinyscale = get_visual_scale(tiny) * GetSizeFromBoundingBox(tiny);

					if (tiny->IsDead()) {
						tinyscale *= 0.4f;
					}

					if (TinyCalamityActionBoostActive(giant)) {
						giantscale *= 6.0f;
					}
					float sizedifference = giantscale / tinyscale;

					if (sizedifference < 1.2f) {
						return false; // terminate task
					}
					else if (sizedifference > 1.2f && sizedifference < 3.0f) {
						StaggerActor(giant, tiny, 0.25f * sizedifference);
						return false; //Only Stagger
					}
				}

				float timeMultiplier = 1.0f / std::max(Time::GGTM(), 1.0e-4f);
				ApplyManualHavokImpulse(tiny, direction.x, direction.y, direction.z, speed * 2.0f * a_power * timeMultiplier);

				return false;
			}
			return true;
		});
	}

	void PushTowards(Actor* a_source, Actor* a_target, NiAVObject* a_bone, float a_power, bool a_doSizeCheck) {
		if (!a_source || !a_target || !a_bone) {
			return;
		}

		NiPoint3 startCoords = a_bone->world.translate;
		std::string boneName = a_bone->name.c_str();
		if (boneName.empty()) {
			return;
		}

		ActorHandle tinyHandle = a_target->CreateRefHandle();
		ActorHandle giantHandle = a_source->CreateRefHandle();

		PushActorAway(a_source, a_target, 1.0f);

		double startTime = Time::WorldTimeElapsed();

		std::string name = std::format("PushTowards_{}_{}", a_source->formID, a_target->formID);
		std::string TaskName = std::format("PushTowards_Job_{}_{}", a_source->formID, a_target->formID);
		// Do this next frame (or rather until some world time has elapsed)
		TaskManager::Run(name, [=](auto& update) {
			if (!giantHandle) {
				return false;
			}
			if (!tinyHandle) {
				return false;
			}
			Actor* giant = giantHandle.get().get();
			if (!giant) {
				return false;
			}

			if (!giant->Is3DLoaded()) {
				return true;
			}
			if (!giant->GetCurrent3D()) {
				return true;
			}

			double endTime = Time::WorldTimeElapsed();

			if ((endTime - startTime) > 1e-4) {

				auto currentBone = find_node(giant, boneName);
				if (!currentBone) {
					return false;
				}

				NiPoint3 endCoords = currentBone->world.translate;

				//log::info("Passing coords: Start: {}, End: {}", Vector2Str(startCoords), Vector2Str(endCoords));
				// Because of delayed nature (and because coordinates become constant once we pass them to TaskManager)
				// i don't have any better idea than to do it through task + task, don't kill me
				PushTowards_Task(giantHandle, tinyHandle, startCoords, endCoords, TaskName, a_power, a_doSizeCheck);
				return false;
			}
			return true;
		});
	}

	void StaggerOr(Actor* a_source, Actor* a_target) {
		if (a_target->IsDead()) {
			return;
		}
		if (InBleedout(a_target)) {
			return;
		}
		if (IsBeingHeld(a_source, a_target)) {
			return;
		}
		if (!AllowStagger(a_target)) {
			return;
		}

		float giantSize = get_visual_scale(a_source);
		float tinySize = get_visual_scale(a_target);

		if (TinyCalamityActionBoostActive(a_source)) {
			giantSize += 1.0f;
		}
		if (a_target->IsPlayerRef() && TinyCalamityActionBoostActive(a_target)) {
			tinySize += 1.25f;
		}

		float sizedifference = giantSize / tinySize;
		float sizedifference_tinypov = tinySize / giantSize;

		int ragdollchance = RandomInt(0, 30);
		if ((giantSize >= 2.0f || AnimationVars::Tiny::IsBeingGrinded(a_target)) && !IsRagdolled(a_target) && sizedifference > 2.8f && ragdollchance < 4.0f * sizedifference) { // Chance for ragdoll. Becomes 100% at high scales
			PushActorAway(a_source, a_target, 1.0f); // Ragdoll
		}
		else if (sizedifference > 1.25f) { // Always Stagger
			AnimationVars::General::SetGiantessScale(a_target, sizedifference_tinypov); // enable stagger just in case
			float push = std::clamp(0.25f * (sizedifference - 0.25f), 0.25f, 1.0f);
			StaggerActor(a_source, a_target, push);
		}
	}

	void Utils_PushCheck(Actor* giant, Actor* tiny, float force) {
		if (giant && tiny) {
			if (tiny->Is3DLoaded() && giant->Is3DLoaded()) {
				bool isdamaging = IsActionOnCooldown(tiny, CooldownSource::Push_Basic);
				if (!isdamaging && (force >= 0.12f || AnimationVars::Action::IsFootGrinding(giant))) {
					//log::info("Check passed, pushing {}, force: {}", tiny->GetDisplayFullName(), force);
					StaggerOr(giant, tiny);
					ApplyActionCooldown(tiny, CooldownSource::Push_Basic);
				}
			}
		}
	}
}
