#pragma once

#include "Proton/Events/Event.h"

namespace proton {

	class AppLayer
	{
	public:
		virtual ~AppLayer() = default;

		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(float timestep) {}
		virtual void OnEvent(Event& event) {}
		virtual void OnImGuiRender() {}
	};

}
