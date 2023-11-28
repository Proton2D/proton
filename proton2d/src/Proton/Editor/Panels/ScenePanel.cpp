#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/ScenePanel.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Scene/SceneManager.h"
#include "Proton/Utils/Utils.h"

#include <imgui.h>


namespace proton {

	void ScenePanel::OnImGuiRender()
	{
		ImGui::Begin("Scene");
		ImGui::Dummy({ 0, 1.0f });

		// Scene name text
		std::string sceneText = m_ActiveScene->m_SceneFilepath;
		if (m_SavedSceneTextTimer > 0.0f)
			sceneText = "[Saved] " + sceneText;
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(sceneText.c_str()).x) / 2);
		ImGui::Text(sceneText.c_str());
		ImGui::Dummy({ 0, 3 });
		
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() 
			- (m_ActiveScene->m_SceneState == SceneState::Stop ? 75 : 160)) / 2.0f);

		// Play / stop / resume buttons
		if (m_ActiveScene->m_SceneState != SceneState::Stop)
		{
			if (ImGui::Button("Stop", { 75, 30 }))
			{
				std::string filepath = m_ActiveScene->GetFilepath();
				if (filepath.size())
				{
					SceneManager::EditorLoadFromCache(filepath);
					SceneManager::SetActiveScene(filepath);
				}
			}

			ImGui::SameLine();
			if (m_ActiveScene->m_SceneState == SceneState::Paused)
			{
				if (ImGui::Button("Resume", { 75, 30 }))
					m_ActiveScene->Pause(false);
			}
			else
			{
				if (ImGui::Button("Pause", { 75, 30 }))
					m_ActiveScene->Pause(true);
			}
		}
		else
		{
			if (ImGui::Button("Play", { 75, 30 }))
				m_ActiveScene->BeginPlay();
		}

		ImGui::Dummy({ 0, 5 });
		ImGui::Separator();
		ImGui::Dummy({ 0, 2 });

		// Scenes loaded in memory view
		DrawSceneMemoryView();

		ImGui::End();
	}

	void ScenePanel::OnUpdate(float ts)
	{
		m_SavedSceneTextTimer = glm::max(m_SavedSceneTextTimer - ts, 0.0f);
	}

	void ScenePanel::DrawSceneMemoryView()
	{
		if (ImGui::TreeNodeEx("Memory view", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Dummy({ 0, 2 });
			for (auto& [scenePath, scenePtr] : SceneManager::s_Instance->m_Scenes)
			{
				ImGui::Text(scenePath.c_str());
				bool isActive = m_ActiveScene == scenePtr;

				ImGui::SameLine(ImGui::GetWindowWidth() - (isActive ? 69 : 160));
				if (!isActive && ImGui::Button(("Set active##" + scenePath).c_str()))
					SceneManager::SetActiveScene(scenePath);

				if (isActive)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 1.0f, 0.2f, 1.0f });
					ImGui::Text("Active");
					ImGui::PopStyleColor();
					ImGui::Dummy({ 0, 2 });
				}
				else
				{
					ImGui::SameLine();
					if (ImGui::Button(("Unload##" + scenePath).c_str()))
					{
						SceneManager::Unload(scenePath);
						break;
					}
				}
			}
			ImGui::TreePop();
		}
	}

}
#endif
