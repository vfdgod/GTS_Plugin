#include "Hooks/Projectile/Projectiles.hpp"
#include "Hooks/Util/HookUtil.hpp"

using namespace GTS;

namespace {

	void ScaleProjectile(Projectile* projectile) {

		if (!projectile) {
			return;
		}

		auto node = projectile->Get3D2();
		if (!node) {
			node = projectile->Get3D1(false);
		}
		if (!node) {
			node = projectile->Get3D1(true);
		}
		if (!node) {
			node = projectile->Get3D();
		} 
		if (node) {
			//log::info("Node found");
			auto owner = projectile->GetProjectileRuntimeData().shooter;
			if (owner) {
				//log::info("Owner found");
				auto owner_get = skyrim_cast<Actor*>(owner.get().get());
				if (owner_get) {
					//log::info("Owner_get found: {}", owner_get->GetDisplayFullName());
					const float scaling = std::clamp(get_visual_scale(owner_get), 0.02f, 1.0f); // not bigger than 1.0x
					node->local.scale *= scaling;
					projectile->GetProjectileRuntimeData().scale *= scaling;
				}
			}
		}
	}
}

namespace Hooks {

	struct Handle3DLoaded {

		static constexpr size_t funcIndex = 0xC0;

		// Scales projectiles once (when it first spawns). Affects only visuals.
		template<int ID>
		static void thunk(RE::Projectile* a_this) {

			func<ID>(a_this);

			{
				GTS_PROFILE_ENTRYPOINT_UNIQUE("Projectile::Handle3DLoaded", ID);
				ScaleProjectile(a_this);
			}

		}

		template<int ID>
		FUNCTYPE_VFUNC_UNIQUE func;
	};

	void Hook_Projectiles::Install() {

		logger::info("Installing Projectile Hooks...");

		// Projectile 3D loaded
		stl::write_vfunc_unique<Handle3DLoaded, 0>(VTABLE_ArrowProjectile[0]);
		//stl::write_vfunc_unique<Handle3DLoaded, 1>(VTABLE_MissileProjectile[0]);
		//stl::write_vfunc_unique<Handle3DLoaded, 2>(VTABLE_BeamProjectile[0]);
		//stl::write_vfunc_unique<Handle3DLoaded, 3>(VTABLE_BarrierProjectile[0]);
		//stl::write_vfunc_unique<Handle3DLoaded, 4>(VTABLE_ConeProjectile[0]);
		//stl::write_vfunc_unique<Handle3DLoaded, 5>(VTABLE_FlameProjectile[0]);
	}

}
