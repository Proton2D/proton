#pragma once

namespace proton {

	class Scene;

	/*
	* SceneManager class methods use scene filepaths - "scenePath" 
	* (realative to "scenes" directory) without ".scene.json" extension
	* as scene identifiers (keys in map storage)
	*/
	class SceneManager
	{
	public:
		static Scene* GetScene(const std::string& scenePath);
		static Scene* GetActiveScene();
		static Scene* SetActiveScene(const std::string& scenePath);

		static Scene* Load(const std::string& scenePath);
		static void Unload(const std::string& scenePath);
		static bool IsLoaded(const std::string& scenePath);

		static void SaveSceneAs(const std::string& scenePath, const std::string& newSenePath);

	private:
		static void Init();
		static Scene* CreateEmptyScene(const std::string& scenePath = "<Unsaved scene>");

	private:
		static SceneManager* s_Instance;

		Scene* m_ActiveScene = nullptr;
		std::map<std::string, Shared<Scene>> m_Scenes;

		friend class Application;
		friend class ToolbarPanel;

		friend class EditorLayer;
		friend class EditorMenuBar;
		friend class SceneViewportPanel;
	};
}
