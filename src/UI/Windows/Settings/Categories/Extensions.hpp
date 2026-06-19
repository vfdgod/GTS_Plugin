#pragma once

#include "UI/Core/ImCategorySplit.hpp"

namespace GTS {

	class CategoryExtensions final : public ImCategorySplit {
		public:
		CategoryExtensions();
		void DrawLeft() override;
		void DrawRight() override;
	};

}
