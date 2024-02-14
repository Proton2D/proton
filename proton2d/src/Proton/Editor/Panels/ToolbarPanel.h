#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/EditorPanel.h"

namespace proton {

	class ToolbarPanel : public EditorPanel
	{
	public:
		virtual void OnImGuiRender() override;

	private:
		void DrawSceneTabBar();

	};

}
#endif // PT_EDITOR
