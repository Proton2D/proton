#pragma once
#ifdef PT_EDITOR
#include "Proton/Scene/Entity.h"

namespace proton {

	class Scene;

	// Abstract class for all editor panels
	class EditorPanel
	{
	public:
		virtual ~EditorPanel() = default;

		virtual void OnCreate() {};
		virtual void OnImGuiRender() = 0;
		virtual void OnUpdate(float ts) {};
		virtual void OnEvent(Event& event) {};
	
	protected:
		Scene* m_ActiveScene = nullptr;
		Entity m_SelectedEntity;

		friend class EditorLayer;
	};

}
#endif // PT_EDITOR
