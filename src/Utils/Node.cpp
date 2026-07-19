
#include "Utils/Node.hpp"

using namespace RE;

namespace GTS {

	constexpr int loop_threshold = 256;

	NiAVObject* find_node(Actor* actor, std::string_view node_name, bool first_person) {
		if (!actor || !actor->Is3DLoaded()) {
			return nullptr;
		}
		auto model = actor->Get3D(first_person);
		if (!model) {
			return nullptr;
		}
		auto node_lookup = model->GetObjectByName(node_name);
		if (node_lookup) {
			return node_lookup;
		}

		// Game lookup failed we try and find it manually
		std::deque<NiAVObject*> queue;
		queue.push_back(model);

        int counter = 0;

		while (!queue.empty()) {
			auto currentnode = queue.front();
			queue.pop_front();

			counter += 1;
			try {
				if (currentnode) {
					auto ninode = currentnode->AsNode();
					if (ninode) {
						for (auto &child : ninode->GetChildren()) {
							// Bredth first search
							if (child) {
								queue.push_back(child.get());
							}
							// Depth first search
							//queue.push_front(child.get());
						}
					} 
					// Do smth
					if (currentnode->name == node_name) {
						logger::trace("Found bone: {}", node_name);
						return currentnode;
					}

					if (counter > loop_threshold) {
						//log::info("Counter {} on {} is > than size {}", counter, actor->GetDisplayFullName(), queue.size());
						queue.clear();
						return nullptr;
					}
				}
			}
			catch (const std::overflow_error& e) {
				logger::warn("Overflow: {}", e.what());
				return nullptr;
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				logger::warn("Underflow: {}", e.what());
				return nullptr;
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				logger::warn("Exception: {}", e.what());
				return nullptr;
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				logger::warn("Exception Other");
				return nullptr;
				}
			}

		return nullptr;
	}


	NiAVObject* find_object_node(TESObjectREFR* object, std::string_view node_name) { // Used inside Looting.cpp only so far
		if (!object) {
			return nullptr;
		}
		auto model = object->GetCurrent3D();
		if (!model) {
			return nullptr;
		}
		auto node_lookup = model->GetObjectByName(node_name);
		if (node_lookup) {
			return node_lookup;
		}

		// Game lookup failed we try and find it manually
		std::deque<NiAVObject*> queue;
		queue.push_back(model);
		
		int counter = 0;

		while (!queue.empty()) {
			auto currentnode = queue.front();
			queue.pop_front();

			counter += 1;

			try {
				if (currentnode) {
					auto ninode = currentnode->AsNode();
					if (ninode) {
						for (auto &child : ninode->GetChildren()) {
							// Bredth first search
							if (child) {
								queue.push_back(child.get());
							}
							// Depth first search
							//queue.push_front(child.get());
						}
					}
					// Do smth
					if (currentnode->name == node_name) {
						return currentnode;
					} else if (counter > loop_threshold) {
						queue.clear();
						return nullptr;
					}
				}
			}
			catch (const std::overflow_error& e) {
				logger::warn("Overflow: {}", e.what());
				return nullptr;
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				logger::warn("Underflow: {}", e.what());
				return nullptr;
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				logger::warn("Exception: {}", e.what());
				return nullptr;
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				logger::warn("Exception Other");
				return nullptr;
			}
		}

		return nullptr;
	}

	NiAVObject* find_node_any(Actor* actor, std::string_view node_name) {
		NiAVObject* result = nullptr;
		for (auto person: {false, true}) {
			result = find_node(actor, node_name, person);
			if (result) {
				break;
			}
		}
		return result;
	}

	BSBound* get_bound(Actor* actor) {
		// This is the bound on the NiExtraNodeData
		if (!actor) {
			return nullptr;
		}
		auto model = actor->Get3D(false);
		if (model) {
			auto extra_bbx = model->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				return bbx;
			}
		}
		auto model_first = actor->Get3D(true);
		if (model_first) {
			auto extra_bbx = model_first->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				return bbx;
			}
		}
		return nullptr;
	}

	NiPoint3 get_bound_values(Actor* actor) {
		NiPoint3 result = NiPoint3(22.0f, 14.0f, 64.0f); // Default human scale that we return if actor for some reason doesn't have BBX data
		if (actor) {
			/*if (IsHumanoid(actor)) { // There's a mod that alters default Human BBX data 
				//  Which leads to messed up humanoid size numbers (Such as 3m instead of 1.8m)
				// https://www.nexusmods.com/skyrimspecialedition/mods/161116
				// Install option: "Bigger XPMSSE Skeleton Collision Bounds"
				// Fix: always use expected human size for humanoids, regardless of true BBX value
				return result;
			}*/
			auto bound = get_bound(actor);
			if (bound) {
				result = bound->extents;
			}
		}
		return result;
	}

	NiAVObject* get_bumper(Actor* actor) {
		std::string node_name = "CharacterBumper";
		return find_node(actor, node_name);
	}

	void update_node(NiAVObject* node) {
		if (node) {
			if (State::OnMainThread()) {
				NiUpdateData ctx;
				node->UpdateWorldData(&ctx);
			} else {
				node->IncRefCount();
				auto task = SKSE::GetTaskInterface();
				task->AddTask([node]() {
					if (node) {
						NiUpdateData ctx;
						node->UpdateWorldData(&ctx);
						node->DecRefCount();
					}
				});
			}
		}
	}

	std::vector<NiAVObject*> GetModelsForSlot(Actor* actor, BGSBipedObjectForm::BipedObjectSlot slot) {
		
		enum {
			k3rd,
			k1st,
			kTotal
		};

		std::vector<NiAVObject*> result = {};
		if (actor) {
			auto armo = actor->GetWornArmor(slot);
			if (armo) {
				auto arma = armo->GetArmorAddonByMask(actor->GetRace(), slot);
				if (arma) {
					char addonString[MAX_PATH]{ '\0' };
					arma->GetNodeName(addonString, actor, armo, -1);
					for (auto first: {true, false}) {
						auto node = find_node(actor, addonString, first);
						if (node) {
							result.push_back(node);
						}
					}
				}
			}
		}
		return result;
	}

	void VisitNodes(NiAVObject* root, const std::function<bool(NiAVObject& a_obj)>& a_visitor) {
		std::deque<NiAVObject*> queue;
		queue.push_back(root);

		int counter = 0;

		while (!queue.empty()) {
			auto currentnode = queue.front();
			queue.pop_front();

			counter += 1;

			if (currentnode) {
				auto ninode = currentnode->AsNode();
				if (ninode) {
					for (auto &child : ninode->GetChildren()) {
						// Bredth first search
						if (child) {
							queue.push_back(child.get());
						}
						// Depth first search
						//queue.push_front(child.get());
					}
				}
				if (counter > loop_threshold) {
					queue.clear();
					return;
				}
				if (!a_visitor(*currentnode)) {
					return;
				}
			}
		}
	}
}
