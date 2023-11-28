#pragma once

#include "Proton/Core/KeyCodes.h"

namespace proton {


	class Input 
	{
	public:
		static bool IsKeyPressed(int keyCode) { return s_Instance->Impl_IsKeyPressed(keyCode); }
		static glm::vec2 GetMousePosition()   { return s_Instance->Impl_GetMousePosition();    }

	protected:
		// Implementation per platform
		virtual bool Impl_IsKeyPressed(int keyCode) = 0;
		virtual glm::vec2 Impl_GetMousePosition() = 0;

	private:
		static Input* s_Instance;

		friend class Application;
	};

}
