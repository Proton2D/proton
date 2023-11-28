#pragma once
#include "Proton/Core/Input.h"

namespace proton {

	class WindowsInput : public Input 
	{
	protected:
		virtual bool Impl_IsKeyPressed(int keyCode) override;
		virtual glm::vec2 Impl_GetMousePosition() override;
	};

}
