#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/MiscellaneousPanel.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Assets/AssetManager.h"
#include "Proton/Core/Application.h"

#include "imgui.h"

namespace proton {

	static constexpr float s_StatsRefreshInterval = 0.2f;

	void MiscellaneousPanel::OnImGuiRender()
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

		ImGui::Begin("Misc");

		if (ImGui::TreeNodeEx("Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::TreeNodeEx("Editor", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Checkbox("Selection outline", &m_SceneViewportPanel->m_ShowSelectionOutline);
				ImGui::Checkbox("Selection collider", &m_SceneViewportPanel->m_ShowSelectionCollider);
				ImGui::Checkbox("Show colliders", &m_SceneViewportPanel->m_ShowAllColliders);
				ImGui::Checkbox("Runtime camera", &EditorLayer::GetCamera().m_UseInRuntime);
				if (ImGui::Button("Reset Camera"))
					m_SceneViewportPanel->ResetCameraPosition();
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx("Application", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushItemWidth(150.0f);
				float timeScale = Application::Get().m_TimeScale;
				if (ImGui::DragFloat("Time scale", &timeScale, 0.01f, 0.0f)
					&& timeScale >= 0.0f)
				{
					Application::Get().m_TimeScale = timeScale;
				}
				ImGui::PopItemWidth();
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Statistics", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Frame time: %f sec. (%.2f FPS)", m_FrameTimeDisplay, m_FPS);

			float max = 0.0f;
			for (uint32_t i = 0; i < s_FrameTimePlotValuesCount; i++)
				max = m_FrameTimeHistory[i] > max ? m_FrameTimeHistory[i] : max;

			ImGui::Text("   max:\n%f\n   avg:\n%f", max, m_AvgFrameTime);
			ImGui::SameLine();
			ImGui::PlotLines("##Frame_Time", m_FrameTimeHistory, s_FrameTimePlotValuesCount, m_FrameTimeValuesOffset, NULL, 0.0f, glm::max(max * 1.1f, 1.0f / 60.0f), ImVec2(0, 80));
			ImGui::Dummy({ 0.0f, 2.0f });

			uint32_t entitiesCount = m_ActiveScene ? m_ActiveScene->GetEntitiesCount() : 0;
			uint32_t scriptedEntitiesCount = m_ActiveScene ? m_ActiveScene->GetScriptedEntitiesCount() : 0;
			ImGui::Text("Entities: %i (%i scripted)", entitiesCount, scriptedEntitiesCount);
			ImGui::Text("OpenGL draw calls: %i", Renderer::GetDrawCallsCount());
			ImGui::TreePop();
		}

		ImGui::End();
		m_FrameCount++;
	}

}

#endif PT_EDITOR
