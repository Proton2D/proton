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
			if (ImGui::Button(FontAwesome_Stop, { 60, 32 }))
			{
				m_ActiveScene->Stop();
			}
			ImGui::SameLine();

			bool paused = m_ActiveScene->m_SceneState == SceneState::Paused;
			if (ImGui::Button(paused ? FontAwesome_Resume : FontAwesome_Pause, { 60, 32 }))
				m_ActiveScene->Pause(!paused);
		}
		else
		{
			if (ImGui::Button(FontAwesome_Play, { 60, 32 }))
			{
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
			const std::string& activeScene = SceneManager::GetActiveScene()->GetFilepath();

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
