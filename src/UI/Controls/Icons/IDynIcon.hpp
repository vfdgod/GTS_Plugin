#pragma once

#include "UI/Core/ImGraphics.hpp"

namespace ImGuiEx {

	class IDynIcon {
		public:

			IDynIcon(const std::string& a_name, uint32_t a_size) :
				m_transform(std::make_unique<GTS::ImGraphics::ImageTransform>()), m_name(a_name), m_size(a_size) {
				m_transform->transformDirection = GTS::ImGraphics::Direction::BottomToTop;
				m_transform->recolorEnabled = true;
				m_transform->gradientFadeEnabled = true;
			}

		virtual ~IDynIcon() = default;

		std::unique_ptr<GTS::ImGraphics::ImageTransform> m_transform = nullptr;
		const std::string m_name = {};
		static constexpr uint32_t m_referenceSize = 64;
		uint32_t m_size = 32;

			void Resize(uint32_t a_size) {
				m_size = a_size;
			}

			protected:
			bool DrawIcon(float a_value, float a_overflow, const std::string& a_text) const;

		};

}
