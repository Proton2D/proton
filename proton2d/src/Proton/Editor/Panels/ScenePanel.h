#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/EditorPanel.h"

namespace proton {

	class ScenePanel : public EditorPanel
	{
	public:
		virtual void OnImGuiRender() override;
		virtual void OnUpdate(float ts) override;

	private:
		void DrawSceneMemoryView();

		void StopSceneSimulation();

	private:
		float m_SavedSceneTextTimer = 0.0f;
	};

}
#endif // PT_EDITOR
