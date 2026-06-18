#include "Papyrus/Papyrus.hpp"
#include "Papyrus/Plugin.hpp"
#include "Papyrus/Scale.hpp"
#include "Papyrus/Height.hpp"
#include "Papyrus/ModEvents.hpp"
#include "Papyrus/TotalControl.hpp"

namespace GTS {

	bool register_papyrus(BSScript::IVirtualMachine* vm) {
		register_papyrus_plugin(vm);
		register_papyrus_scale(vm);
		register_papyrus_height(vm);
		register_papyrus_events(vm);
		register_total_control(vm);
		return true;
	}

}
