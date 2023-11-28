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

	SceneManager::~SceneManager()
	{
		for (auto& [scenePath, scene] : m_Scenes)
			delete scene;
	}

	void SceneManager::Init()
	{
		if (!s_Instance)
		{
			s_Instance = new SceneManager();
		#ifdef PT_EDITOR
			Scene* scene = CreateEmptyScene("<Unsaved scene>");
			s_Instance->m_ActiveScene = scene;
			EditorLayer::SetActiveScene(scene);
		#endif
		}
	}

	bool SceneManager::IsLoaded(const std::string& scenePath)
	{
		return s_Instance->m_Scenes.find(scenePath) != s_Instance->m_Scenes.end();
	}

	Scene* SceneManager::Load(const std::string& scenePath, bool setActive)
	{
		if (scenePath != "<Unsaved scene>")
			PT_CORE_INFO("[SceneManager::Load] file='{}.scene.json'", scenePath);
		std::string filepath = "content/scenes/" + scenePath + ".scene.json";
		Scene* scene = s_Instance->Deserialize(scenePath, filepath);
		if (setActive)
			SetActiveScene(scenePath);
		return scene;
	}

	Scene* SceneManager::EditorLoadFromCache(const std::string& scenePath)
	{
		std::string filepath = "editor/cache/" + 
			(scenePath == "<Unsaved scene>" ? "unsaved_scene" : scenePath) + ".scene.json";
		std::replace(filepath.begin(), filepath.end(), '\\', '_');
		return s_Instance->Deserialize(scenePath, filepath);
	}

	Scene* SceneManager::Deserialize(const std::string& scenePath, const std::string& fullFilepath)
	{
		Scene* scene = CreateEmptyScene(scenePath, false);
		SceneSerializer serializer(scene);
		if (!serializer.Deserialize(fullFilepath))
		{
			PT_CORE_ERROR("[SceneManager::Deserialize] Loading '{}' failed!", fullFilepath);
			return nullptr;
		}
		s_Instance->m_Scenes[scenePath] = scene;
		return scene;
	}

	void SceneManager::Unload(const std::string& scenePath)
	{
		if (scenePath != "<Unsaved scene>")
			PT_CORE_INFO("[SceneManager::Unload] Scene '{}'", scenePath);
		delete s_Instance->m_Scenes.at(scenePath);
		s_Instance->m_Scenes.erase(scenePath);
	}

	// TODO: Refactor this function
	Scene* SceneManager::SetActiveScene(const std::string& scenePath, bool autoLoad)
	{
		if (autoLoad && !IsLoaded(scenePath))
			Load(scenePath);

		if (!IsLoaded(scenePath))
		{
			PT_CORE_ERROR("[SceneManager::SetActiveScene] Scene '{}' not loaded!", scenePath);
			return nullptr;
		}
		Scene* targetScene = s_Instance->m_Scenes.at(scenePath);
#ifdef PT_EDITOR
		// Remove unsaved scene if it's empty
		if (IsLoaded("<Unsaved scene>"))
		{
			Scene* scene = GetScene("<Unsaved scene>");
			if (!scene->m_Registry.size() && s_Instance->m_Scenes.size() > 1)
				Unload("<Unsaved scene>");
		}
		if (s_Instance->m_ActiveScene->GetSceneState() == SceneState::Play)
			targetScene->BeginPlay();

		s_Instance->m_ActiveScene = targetScene;
		EditorLayer::SetActiveScene(targetScene);
		EditorLayer::SelectEntity({});

#else
		s_Instance->m_ActiveScene = targetScene;
		s_Instance->m_ActiveScene->BeginPlay();
#endif
		PT_CORE_INFO("[SceneManager::SetActiveScene] scene='{}'", scenePath);

		Renderer::SetClearColor(s_Instance->m_ActiveScene->m_ClearColor);
		return s_Instance->m_ActiveScene;
	}

	void SceneManager::SaveSceneAs(const std::string& scenePath, const std::string& newScenePath)
	{
		if (!IsLoaded(scenePath))
		{
			PT_CORE_ERROR("[SceneManager::SaveSceneAs] Scene '{}' not loaded!", scenePath);
			return;
		}

		SceneSerializer serializer(s_Instance->m_Scenes.at(scenePath));
		serializer.Serialize("content/scenes/" + newScenePath + ".scene.json");
	}

	void SceneManager::SaveActiveSceneAs(const std::string& scenePath)
	{
		SaveSceneAs(s_Instance->m_ActiveScene->m_SceneFilepath, scenePath);
	}

	void SceneManager::SaveActiveScene()
	{
		std::string filepath = s_Instance->m_ActiveScene->m_SceneFilepath;
		SaveSceneAs(filepath, filepath);
	}

	Scene* SceneManager::GetActiveScene()
	{
		return s_Instance->m_ActiveScene;
	}

	Scene* SceneManager::GetScene(const std::string& scenePath)
	{
		if (!IsLoaded(scenePath))
		{
			PT_CORE_ERROR("[SceneManager::GetScene] Scene not found!");
			return nullptr;
		}
		return s_Instance->m_Scenes.at(scenePath);
	}

	const std::string& SceneManager::GetActiveSceneFilepath()
	{
		return s_Instance->m_ActiveScene->m_SceneFilepath;
	}

	Scene* SceneManager::CreateEmptyScene(const std::string& scenePath, bool addToRegistry)
	{
		Scene* scene = new Scene();
		scene->m_SceneFilepath = scenePath;
		if (IsLoaded(scenePath))
			delete s_Instance->m_Scenes.at(scenePath);
		if (addToRegistry)
			s_Instance->m_Scenes[scenePath] = scene;
		return scene;
	}

}
