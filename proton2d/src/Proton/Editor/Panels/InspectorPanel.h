#pragma once
#ifdef PT_EDITOR

#include "Proton/Editor/Panels/EditorPanel.h"
#include "Proton/Core/Base.h"
#include "Proton/Scene/Entity.h"

namespace proton {

	class InspectorPanel : public EditorPanel
	{
	public:
		virtual void OnImGuiRender() override;

	private:
		void DrawSceneProporties();

		template<typename T>
		void DrawComponentUI(const std::string& name, const std::function<void(T&)>& drawContentFunction);

		friend class EditorLayer;
	};

}

#endif // PT_EDITOR
