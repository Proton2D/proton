#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Editor/Panels/SettingsPanel.h"
#include "Proton/Editor/Panels/SceneViewportPanel.h"
#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Assets/AssetManager.h"
#include "Proton/Core/Application.h"

#include "imgui.h"

namespace proton {

	static constexpr float s_StatsRefreshInterval = 0.2f;

	void SettingsPanel::OnImGuiRender()
	{
		ImGui::Begin("Settings");

		if (ImGui::TreeNodeEx("Editor", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SceneViewportPanel* viewportPanel = EditorLayer::GetSceneViewportPanel();
			ImGui::Dummy({ 0, 2 });
			ImGui::Checkbox("Selection outline", &viewportPanel->m_ShowSelectionOutline);
			ImGui::Checkbox("Selection collider", &viewportPanel->m_ShowSelectionCollider);
			ImGui::Checkbox("Show colliders", &viewportPanel->m_ShowAllColliders);
			ImGui::Checkbox("Runtime camera", &EditorLayer::GetCamera()->m_UseInRuntime);
			ImGui::TreePop();
		}
		ImGui::Dummy({ 0, 5 });
		if (ImGui::TreeNodeEx("Application", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Dummy({ 0, 2 });
			Window& window = Application::Get().GetWindow();
			bool fullscreen = window.IsFullscreen();
			if (ImGui::Checkbox("Fullscreen", &fullscreen))
				window.SetFullscreen(fullscreen);
			bool vsync = Application::Get().GetWindow().IsVSync();
			if (ImGui::Checkbox("VSync", &vsync))
				window.SetVSync(vsync);

			ImGui::PushItemWidth(100.0f);
			float timeScale = Application::Get().m_TimeScale;
			if (ImGui::DragFloat("Time scale", &timeScale, 0.01f, 0.0f)
				&& timeScale >= 0.0f)
			{
				Application::Get().m_TimeScale = timeScale;
			}
			ImGui::PopItemWidth();
			ImGui::TreePop();
		}

		ImGui::End();
	}

}

#endif PT_EDITOR
