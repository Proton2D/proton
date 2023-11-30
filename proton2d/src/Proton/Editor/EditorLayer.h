#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/EditorCamera.h"
#include "Proton/Editor/EditorMenuBar.h"
#include "Proton/Editor/Panels/MiscellaneousPanel.h"
#include "Proton/Editor/Panels/InspectorPanel.h"
#include "Proton/Editor/Panels/SceneHierarchyPanel.h"
#include "Proton/Editor/Panels/SceneViewportPanel.h"
#include "Proton/Editor/Panels/ScenePanel.h"
#include "Proton/Editor/Panels/PrefabPanel.h"

#include "Proton/Core/AppLayer.h"
#include "Proton/Core/Config.h"
#include "Proton/Scene/Entity.h"
#include "Proton/Graphics/Renderer/Framebuffer.h"

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

		static void SetActiveScene(Scene* scene);
		static void SelectEntity(Entity entity);

		static EditorCamera& GetCamera();

	private:
		void BeginImGuiRender();
		void EndImGuiRender();

	private:
		static EditorLayer* s_Instance;
		Scene* m_ActiveScene = nullptr;
		Entity m_SelectedEntity;

		EditorConfig m_Config;
		EditorMenuBar m_MenuBar;

		MiscellaneousPanel m_MiscPanel;
		InspectorPanel m_InspectorPanel;
		SceneHierarchyPanel m_SceneHiearchyPanel;
		ScenePanel m_ScenePanel;
		PrefabPanel m_PrefabPanel;
		SceneViewportPanel m_SceneViewportPanel;

		std::vector<EditorPanel*> m_EditorPanels;

		bool m_EnableViewports = false; // when set to true ImGui windows can be detached from main GLFW window
		bool m_BlockEvents = true;

		friend class Application;
		friend class Scene;

		friend class InspectorPanel;
		friend class MiscellaneousPanel;
		friend class ScenePanel;
		friend class EditorCamera;
		friend class SceneViewportPanel;
	};

}
#endif
