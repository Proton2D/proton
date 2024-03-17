#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/EditorMenuBar.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Editor/Panels/SceneViewportPanel.h"
#include "Proton/Core/Application.h"
#include "Proton/Scene/SceneManager.h"
#include "Proton/Utils/Utils.h"

#include <imgui.h>

namespace proton {

	void EditorMenuBar::OnImGuiRender()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
					OpenScene();

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					Application::Get().Exit();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Editor"))
			{
				if (ImGui::MenuItem("Reset Camera"))
					EditorLayer::GetCamera()->SetPosition({0.0f, 0.0f, 0.0f});
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	static std::string GetSceneFilename(const std::string& filepath)
	{
		std::size_t pos = filepath.find("scenes");
		if (pos != std::string::npos) {
			std::string filename = filepath.substr(pos + 7);
			std::size_t posExt = filepath.find(".scene.json");
			if (posExt != std::string::npos)
				return filename.substr(0, filename.size() - 11);
			return filename;
		}
		return std::string();
	}

	void EditorMenuBar::NewScene()
	{
		Scene* scene = SceneManager::CreateEmptyScene();
		SceneManager::s_Instance->m_ActiveScene = scene;
		EditorLayer::SetActiveScene(scene);
	}

	void EditorMenuBar::OpenScene()
	{
		std::string sceneFile = GetSceneFilename(FileDialogs::OpenFile("scene"));
		if (sceneFile.size())
		{
			SceneManager::Load(sceneFile);
			SceneManager::SetActiveScene(sceneFile);
		}
	}

	void EditorMenuBar::SaveScene()
	{
		Scene* activeScene = SceneManager::GetActiveScene();
		if (!activeScene)
			return;

		if (activeScene->m_SceneFilepath != "<Unsaved scene>")
		{
			const std::string filepath = activeScene->m_SceneFilepath;
			SceneManager::SaveSceneAs(filepath, filepath);
		}
		else
			SaveSceneAs();
	}

	void EditorMenuBar::SaveSceneAs()
	{
		Scene* activeScene = SceneManager::GetActiveScene();
		if (!activeScene)
			return;

		std::string filepath = GetSceneFilename(FileDialogs::SaveFile(".scene.json"));
		if (filepath.size())
		{
			SceneManager::SaveSceneAs(activeScene->m_SceneFilepath, filepath);
			if (activeScene->m_SceneFilepath == "<Unsaved scene>")
			{
				SceneManager::Unload("<Unsaved scene>");
			}
			SceneManager::Load(filepath);
			SceneManager::SetActiveScene(filepath);
		}
	}

}

#endif // PT_EDITOR
