#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/InfoPanel.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Assets/AssetManager.h"
#include "Proton/Core/Application.h"

#include "imgui.h"

namespace proton {

	static constexpr float s_StatsRefreshInterval = 0.2f;

	void InfoPanel::OnImGuiRender()
	{
		m_FrameTime = Application::Get().m_FrameTime;

		if (m_RefreshStatsTimer <= 0.0f)
		{
			m_FrameTimeDisplay = m_FrameTime;

			m_FrameTimeHistory[m_FrameTimeValuesOffset] = m_FrameTime;
			m_FrameTimeValuesOffset = (m_FrameTimeValuesOffset + 1) % s_FrameTimePlotValuesCount;

			m_FPS = m_FrameCount ? (m_FrameCount - 1) / (s_StatsRefreshInterval - m_RefreshStatsTimer) : 0.0f;
			m_FrameCount = 0;

			float sum = 0.0f; int total = 0;
			for (uint32_t i = 0; i < s_FrameTimePlotValuesCount; i++)
			{
				if (m_FrameTimeHistory[i] != 0.0f)
				{
					sum += m_FrameTimeHistory[i];
					total++;
				}
			}

			if (total)
				m_AvgFrameTime = sum / (float)total;

			m_RefreshStatsTimer = s_StatsRefreshInterval;
		}
		else
			m_RefreshStatsTimer -= m_FrameTime;

		ImGui::Begin("Info");

		ImGui::Dummy({ 0, 5 });
		uint32_t entitiesCount = m_ActiveScene ? m_ActiveScene->GetEntitiesCount() : 0;
		uint32_t scriptedEntitiesCount = m_ActiveScene ? m_ActiveScene->GetScriptedEntitiesCount() : 0;
		ImGui::Text("Entities: %i (%i scripted)", entitiesCount, scriptedEntitiesCount);
		ImGui::Text("OpenGL Draw Calls: %i", m_ActiveScene ? Renderer::GetDrawCallsCount() : 0);

		ImGui::Dummy({ 0, 10 });
		ImGui::Text("Frame time: %f sec. (%.2f FPS)", m_FrameTimeDisplay, m_FPS);

		float max = 0.0f;
		for (uint32_t i = 0; i < s_FrameTimePlotValuesCount; i++)
			max = m_FrameTimeHistory[i] > max ? m_FrameTimeHistory[i] : max;

		ImGui::Text("   max:\n%f\n   avg:\n%f", max, m_AvgFrameTime);
		ImGui::SameLine();
		ImGui::PlotLines("##Frame_Time", m_FrameTimeHistory, s_FrameTimePlotValuesCount, m_FrameTimeValuesOffset, NULL, 0.0f, glm::max(max * 1.1f, 1.0f / 60.0f), ImVec2(0, 80));
		ImGui::End();
		m_FrameCount++;
	}

}

#endif PT_EDITOR
