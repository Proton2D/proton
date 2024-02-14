#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/EditorPanel.h"

namespace proton {

	class SceneViewportPanel;

	class InfoPanel : public EditorPanel
	{
	public:
		virtual void OnImGuiRender() override;

	private:
		float m_FrameTime = 0.0f;
		float m_FrameTimeDisplay = 0.0f;
		float m_AvgFrameTime = 0.0f;

		uint32_t m_FrameCount = 0;
		float m_FPS = 0.0f;

		static constexpr uint32_t s_FrameTimePlotValuesCount = 200;
		float m_FrameTimeHistory[s_FrameTimePlotValuesCount] = {};
		uint32_t m_FrameTimeValuesOffset = 0;

		float m_RefreshStatsTimer = 0.0f;

		friend class Application;
		friend class EditorLayer;
	};

}

#endif // PT_EDITOR
