#include "Systems/Colliders/ColliderData.hpp"
#include "Systems/Colliders/CollidesWith.hpp"

namespace GTS {

	namespace {
		std::vector<hkpWorldObject*> GetWorldObjectsFrom(
			const std::vector<hkpRigidBody*>& a_rigidBodies,
			const std::vector<hkpPhantom*>& a_phantoms
		) {
			std::vector<hkpWorldObject*> entities;
			entities.reserve(a_rigidBodies.size() + a_phantoms.size());

			for (auto* rb : a_rigidBodies) {
				entities.push_back(rb);
			}
			for (auto* phantom : a_phantoms) {
				entities.push_back(phantom);
			}

			return entities;
		}
	}

	void ColliderData::Activate() {
		logger::info("Activate RBs");
		for (auto rb: GetRigidBodies()) {
			if (!rb) {
				continue;
			}
			logger::info("  - Activating");
			rb->SetMotionType(hkpMotion::MotionType::kCharacter, hkpEntityActivation::kDoActivate, hkpUpdateCollisionFilterOnEntityMode::kFullCheck);
		}
	}

	void ColliderData::UpdateCollisionFilter() {
		for (auto ent: GetRigidBodies()) {
			if (ent) {
				if (ent->world) {
					ent->world->UpdateCollisionFilterOnEntity(ent, hkpUpdateCollisionFilterOnEntityMode::kFullCheck, hkpUpdateCollectionFilterMode::kIncludeCollections);
				}
			}
		}
		for (auto ent: GetPhantoms()) {
			if (ent) {
				if (ent->world) {
					ent->world->UpdateCollisionFilterOnPhantom(ent, hkpUpdateCollectionFilterMode::kIncludeCollections);
				}
			}
		}
	}

	void ColliderData::AddRB(hkpRigidBody* rb) {
		if (!rb) {
			return;
		}
		this->rbs.try_emplace(rb, hkRefPtr(rb));
	}

	void ColliderData::AddPhantom(hkpPhantom* phantom) {
		if (!phantom) {
			return;
		}
		this->phantoms.try_emplace(phantom, hkRefPtr(phantom));
	}

	std::vector<ColliderData*> ColliderData::GetChildren() {
		return {};
	}

	std::vector<hkpRigidBody*> ColliderData::GetRigidBodies() {
		std::vector<hkpRigidBody*> entities = {};
		for (auto& rb : this->rbs | std::views::values) {
			entities.push_back(rb.get());
		}
		for (auto& child: GetChildren()) {
			for (auto& ent: child->GetRigidBodies()) {
				entities.push_back(ent);
			}
		}
		return entities;
	}
	
	std::vector<hkpPhantom*> ColliderData::GetPhantoms() {
		std::vector<hkpPhantom*> entities = {};
		for (auto& ph : this->phantoms | std::views::values) {
			entities.push_back(ph.get());
		}
		for (auto& child: GetChildren()) {
			for (auto& ent: child->GetPhantoms()) {
				entities.push_back(ent);
			}
		}
		return entities;
	}

	std::vector<hkpWorldObject*> ColliderData::GetWorldObjects() {
		std::vector<hkpWorldObject*> entities = {};
		for (auto& rb: GetRigidBodies()) {
			entities.push_back(rb);
		}
		for (auto& ph: GetPhantoms()) {
			entities.push_back(ph);
		}
		return entities;
	}

	void ColliderData::DisableCollisions() {
		const auto rigidBodies = GetRigidBodies();
		const auto phantoms = GetPhantoms();
		const auto worldObjects = GetWorldObjectsFrom(rigidBodies, phantoms);

		for (auto& rb: rigidBodies) {
			if (!rb) {
				continue;
			}
			// Disable gravity
			// log::info("Disable gravity (was {})", rb->motion.gravityFactor);
			rb->motion.gravityFactor = 0.0f;
			rb->motion.SetMassInv(0.0f);
		}

		for (auto& ent: worldObjects) {
			if (!ent) {
				continue;
			}
			auto collidable = ent->GetCollidable();
			if (collidable) {
				// log::info("- Disable collision");
				// log::info("Current info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
				// log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);

				// Change collides with
				if (GetCollidesWith(ent) == COL_LAYER::kCharController) {
					SetCollidesWith(ent, COL_LAYER::kNonCollidable);
				}

				// log::info("    New info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
				// log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
			}
		}
	}

	void ColliderData::EnableCollisions() {
		const auto rigidBodies = GetRigidBodies();
		const auto phantoms = GetPhantoms();
		const auto worldObjects = GetWorldObjectsFrom(rigidBodies, phantoms);

		for (auto& rb: rigidBodies) {
			if (!rb) {
				continue;
			}
			// Enable gravity
			rb->motion.gravityFactor = 1.0f;
			rb->motion.SetMassInv(1.0f);
		}

		for (auto& ent: worldObjects) {
			if (!ent) {
				continue;
			}
			auto collidable = ent->GetCollidable();
			if (collidable) {
				// log::info("- Enabling collision");
				// log::info("Current info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
				// log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);

				// Change collides with
				if (GetCollidesWith(ent) == COL_LAYER::kNonCollidable) {
					SetCollidesWith(ent, COL_LAYER::kCharController);
				}

				// log::info("    New info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
				// log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
			}
		}
	}
}
