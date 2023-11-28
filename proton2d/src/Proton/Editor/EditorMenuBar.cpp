#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/EditorMenuBar.h"
#include "Proton/Editor/EditorLayer.h"
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
		Scene* scene = SceneManager::CreateEmptyScene("<Unsaved scene>");
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
		if (SceneManager::GetActiveScene()->m_SceneFilepath != "<Unsaved scene>")
		{
			SceneManager::SaveActiveScene();
		}
		else
			SaveSceneAs();
	}

	void EditorMenuBar::SaveSceneAs()
	{
		std::string filepath = GetSceneFilename(FileDialogs::SaveFile(".scene.json"));
		if (filepath.size())
		{
			SceneManager::SaveActiveSceneAs(filepath);
			if (SceneManager::GetActiveScene()->m_SceneFilepath == "<Unsaved scene>")
			{
				SceneManager::Unload("<Unsaved scene>");
			}
			SceneManager::Load(filepath);
			SceneManager::SetActiveScene(filepath);
		}
	}

}

#endif // PT_EDITOR
