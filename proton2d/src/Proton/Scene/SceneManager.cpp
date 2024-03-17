#include "ptpch.h"
#include "Proton/Scene/SceneManager.h"
#include "Proton/Scene/Scene.h"
#include "Proton/Assets/SceneSerializer.h"
#include "Proton/Core/Application.h"
#include "Proton/Graphics/Renderer/Renderer.h"

#ifdef PT_EDITOR
#include "Proton/Editor/EditorLayer.h"
#endif

namespace proton {

	SceneManager* SceneManager::s_Instance = nullptr;

	void SceneManager::Init()
	{
		if (!s_Instance)
			s_Instance = new SceneManager();
	}

	Scene* SceneManager::GetScene(const std::string& scenePath)
	{
		return s_Instance->m_Scenes.at(scenePath).get();
	}

	Scene* SceneManager::GetActiveScene()
	{
		return s_Instance->m_ActiveScene;
	}

	Scene* SceneManager::SetActiveScene(const std::string& scenePath)
	{
		if (!IsLoaded(scenePath) && !Load(scenePath))
		{
			PT_CORE_ERROR_FUNCSIG("Scene '{}' not loaded!", scenePath);
			return nullptr;
		}

		PT_CORE_INFO_FUNCSIG("scene='{}'", scenePath);
		Scene* targetScene = GetScene(scenePath);
		s_Instance->m_ActiveScene = targetScene;
		Renderer::SetClearColor(s_Instance->m_ActiveScene->m_ClearColor);

	#ifdef PT_EDITOR
		EditorLayer::SetActiveScene(targetScene);
	#endif
		return s_Instance->m_ActiveScene;
	}

	Scene* SceneManager::Load(const std::string& scenePath)
	{
		PT_CORE_INFO_FUNCSIG("file='{}.scene.json'", scenePath);
		Shared<Scene> scene = MakeShared<Scene>(std::string(), scenePath);
		SceneSerializer serializer(scene.get());

		std::string filepath = "content/scenes/" + scenePath + ".scene.json";
		if (!serializer.Deserialize(filepath))
		{
			PT_CORE_ERROR_FUNCSIG("Loading '{}' failed!", filepath);
			return nullptr;
		}
		s_Instance->m_Scenes[scenePath] = scene;
		return scene.get();
	}

	void SceneManager::Unload(const std::string& scenePath)
	{
		Scene* scene = GetScene(scenePath);
		if (!scene)
		{
			PT_CORE_ERROR_FUNCSIG("scene='{}' not found", scenePath);
			return;
		}

		std::string name = scenePath;
		PT_CORE_INFO_FUNCSIG("scene='{}'", name);

		bool isActive = scene == s_Instance->m_ActiveScene;
		s_Instance->m_Scenes.erase(scenePath);
		if (isActive)
		{
			if (s_Instance->m_Scenes.size())
			{
				// If unloaded active scene, switch to first scene
				SetActiveScene(s_Instance->m_Scenes.begin()->first);
			}
			else
			{
				s_Instance->m_ActiveScene = nullptr;
			#ifdef PT_EDITOR
				EditorLayer::SetActiveScene(nullptr);
			#endif
			}
		}
	}

	bool SceneManager::IsLoaded(const std::string& scenePath)
	{
		return s_Instance->m_Scenes.find(scenePath) != s_Instance->m_Scenes.end();
	}

	void SceneManager::SaveSceneAs(const std::string& scenePath, const std::string& newScenePath)
	{
		if (!IsLoaded(scenePath))
		{
			PT_CORE_ERROR_FUNCSIG("Scene '{}' not loaded!", scenePath);
			return;
		}

		SceneSerializer serializer(GetScene(scenePath));
		serializer.Serialize("content/scenes/" + newScenePath + ".scene.json");
	}

	Scene* SceneManager::CreateEmptyScene(const std::string& scenePath)
	{
		Shared<Scene> scene = MakeShared<Scene>("Unnamed Scene", "<Unsaved scene>");
		s_Instance->m_Scenes["<Unsaved scene>"] = scene;
		return scene.get();
	}

}
