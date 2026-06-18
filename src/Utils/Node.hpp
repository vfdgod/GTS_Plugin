#pragma once

namespace GTS {

	NiAVObject* find_node(Actor* actor, std::string_view node_name, bool first_person = false);
	NiAVObject* find_object_node(TESObjectREFR* object, std::string_view node_name);
	NiAVObject* find_node_any(Actor* actor, std::string_view node_name);

	BSBound* get_bound(Actor* actor);
	NiPoint3 get_bound_values(Actor* actor);
	
	NiAVObject* get_bumper(Actor* actor);
	void update_node(NiAVObject* node);

	std::vector<NiAVObject*> GetModelsForSlot(Actor* actor, BGSBipedObjectForm::BipedObjectSlot slot);
	void VisitNodes(NiAVObject* root, const std::function<bool(NiAVObject& a_obj)>& a_visitor);
	
	template<typename T>
	void VisitExtraData(NiAVObject* root, std::string_view name, std::function<bool(NiAVObject& a_obj, T& data)> a_visitor) {
		VisitNodes(root, [&root, &name, &a_visitor](NiAVObject& node) {
			NiExtraData* extraData = node.GetExtraData(name);
			if (extraData) {
				T* targetExtraData = netimmerse_cast<T*>(extraData);
				if (targetExtraData) {
					if (!a_visitor(node, *targetExtraData)) {
						return false;
					}
				}
			}
			return true;
		});
	}
}
