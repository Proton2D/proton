#pragma once

#include "Proton/Core/KeyCodes.h"

namespace proton {

	class Input 
	{
	public:
		static bool IsKeyPressed(int keyCode);
		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
	};

}
