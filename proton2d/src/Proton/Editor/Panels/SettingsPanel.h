#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/EditorPanel.h"

namespace proton {

	class SceneViewportPanel;

	class SettingsPanel : public EditorPanel
	{
	public:
		virtual void OnImGuiRender() override;

		friend class Application;
		friend class EditorLayer;
	};

}

#endif // PT_EDITOR
