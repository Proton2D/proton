#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/EditorCamera.h"
#include "Proton/Editor/EditorMenuBar.h"
#include "Proton/Core/AppLayer.h"
#include "Proton/Core/Config.h"
#include "Proton/Scene/Entity.h"
#include "Proton/Graphics/Renderer/Framebuffer.h"

struct ImFont;

namespace proton {

	class EditorPanel;
	class SceneViewportPanel;

	class EditorLayer : AppLayer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnUpdate(float ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

		static EditorLayer* Get() { return s_Instance; }
		static EditorCamera* GetCamera();
		static SceneViewportPanel* GetSceneViewportPanel();
		static ImFont* GetFontAwesome();
		static ImFont* GetSmallFont();

		static void SetActiveScene(Scene* scene);
		static void SelectEntity(Entity entity);

	private:
		void SetupFonts();
		void SetupThemeStyle();
		void SetupImGuiViewports();
		void InitializeImGui();

		void BeginImGuiRender();
		void EndImGuiRender();

		void OnBeginSceneSimulation(Scene* scene);
		void OnStopSceneSimulation(Scene* scene);

	private:
		static EditorLayer* s_Instance;

		bool m_EnableViewports = false; // true: ImGui windows can be detached from main GLFW window

		Scene* m_ActiveScene = nullptr;
		Entity m_SelectedEntity;

		std::unordered_map<std::string, Shared<Scene>> m_SceneBackup;

		EditorConfig m_Config;
		EditorMenuBar m_MenuBar;

		std::vector<EditorPanel*> m_EditorPanels;
		bool m_BlockEvents = true;

		friend class Application;
		friend class Scene;

		friend class SceneViewportPanel;
		friend class InspectorPanel;
		friend class SettingsPanel;
		friend class InfoPanel;
		friend class ToolbarPanel;
		friend class EditorCamera;
		friend class EditorMenuBar;
	};

}
#endif
