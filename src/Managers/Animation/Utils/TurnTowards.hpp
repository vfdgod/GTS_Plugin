#pragma once

namespace GTS {

	template<typename T, typename U>
	bool FaceSame(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return false;
		}
		auto giantAngle = giant->data.angle.z;
		tiny->data.angle.z = giantAngle;
		return true;
	}

	template<typename T, typename U>
	bool FaceOpposite(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return false;
		}
		auto giantAngle = giant->data.angle.z;
		float opposite = static_cast<float>(giantAngle - std::numbers::pi);
		if (opposite < 0.0f) {
			opposite += static_cast<float>(2.0 * std::numbers::pi);
		}
		tiny->data.angle.z = opposite;
		return true;
	}

}
