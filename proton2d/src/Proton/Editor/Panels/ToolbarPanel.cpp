#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/ToolbarPanel.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Scene/SceneManager.h"
#include "Proton/Assets/SceneSerializer.h"
#include "Proton/Physics/PhysicsWorld.h"
#include "Proton/Utils/Utils.h"

#include <imgui.h>

static constexpr const char FontAwesome_Play[]   = u8"\uf04b";
static constexpr const char FontAwesome_Pause[]  = u8"\uf04c";
static constexpr const char FontAwesome_Resume[] = u8"\uf051";
static constexpr const char FontAwesome_Stop[]   = u8"\uf04d";

namespace proton {

	void ToolbarPanel::OnImGuiRender()
	{
		ImGui::Begin("Toolbar", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse);
		
		if (!m_ActiveScene)
		{
			ImGui::End();
			return;
		}

		ImGui::PushFont(EditorLayer::GetFontAwesome());
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (m_ActiveScene->m_SceneState == SceneState::Stop ? 60 : 145)) / 2.0f);

		if (m_ActiveScene->m_SceneState != SceneState::Stop)
		{
			// STOP simulation button
			if (ImGui::Button(FontAwesome_Stop, { 60, 32 }))
			{
				std::string filepath = m_ActiveScene->m_SceneFilepath;
				Scene* scene = SceneManager::GetActiveScene();
				if (scene->m_PhysicsWorld->IsInitialized())
					scene->m_PhysicsWorld->DestroyWorld();
				scene->m_SceneState = SceneState::Stop;
				SceneManager::EditorLoadFromCache(filepath);
				SceneManager::SetActiveScene(filepath);
			}
			ImGui::SameLine();

			// PAUSE / RESUME simulation button
			bool paused = m_ActiveScene->m_SceneState == SceneState::Paused;
			if (ImGui::Button(paused ? FontAwesome_Resume : FontAwesome_Pause, { 60, 32 }))
				m_ActiveScene->Pause(!paused);
		}
		else
		{
			// PLAY simulation button
			if (ImGui::Button(FontAwesome_Play, { 60, 32 }))
			{
				// Temporary solution
				SceneSerializer serializer(m_ActiveScene); 
				std::string filepath = m_ActiveScene->m_SceneFilepath == "<Unsaved scene>" ? "unsaved_scene" : m_ActiveScene->m_SceneFilepath;
				std::replace(filepath.begin(), filepath.end(), '\\', '_');
				serializer.Serialize("editor/cache/" + filepath + ".scene.json");
				m_ActiveScene->BeginPlay();
			}
		}
		ImGui::PopFont();
		DrawSceneTabBar();

		ImGui::End();
	}

	void ToolbarPanel::DrawSceneTabBar()
	{
		
		if (ImGui::BeginTabBar("SceneTabBar", ImGuiTabBarFlags_AutoSelectNewTabs))
		{
			const std::string& activeScene = SceneManager::GetActiveSceneFilepath();

			for (auto& [name, scene] : SceneManager::s_Instance->m_Scenes)
			{
				bool keepOpen = true;
				bool selected = activeScene == name;
				ImGuiTabBarFlags flags = selected ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;

				bool result = ImGui::BeginTabItem(name.c_str(), &keepOpen, flags);

				if (result)
					ImGui::EndTabItem();

				if (ImGui::IsItemClicked() && !selected)
					SceneManager::SetActiveScene(name);

				if (!keepOpen)
				{
					SceneManager::Unload(name);
					break;
				}
			}
			ImGui::EndTabBar();
		}
	}

}
#endif
