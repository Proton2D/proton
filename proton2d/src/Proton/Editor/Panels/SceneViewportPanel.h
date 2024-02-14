#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/EditorPanel.h"
#include "Proton/Graphics/Renderer/Framebuffer.h"

namespace proton {

	class SceneViewportPanel : public EditorPanel
	{
	public:
		virtual void OnCreate() override;
		virtual void OnImGuiRender() override;
		virtual void OnUpdate(float ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		void DrawCollidersAndSelectionOutline();
		void HandleImGuiDragAndDrop();

	private:
		Unique<EditorCamera> m_Camera;

		Shared<Framebuffer> m_Framebuffer;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2] = { { 0.0f, 0.0f }, {0.0f, 0.0f} };
		
		glm::vec2 m_MousePos = { 0.0f, 0.0f };
		glm::vec2 m_SelectionMouseOffset = { 0.0f, 0.0f };
		glm::vec2 m_CameraDragOffset = { 0.0f, 0.0f };

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		bool m_MoveSelectedEntity = false;
		bool m_MoveEditorCamera = false;

		bool m_ShowSelectionOutline = true;
		bool m_ShowSelectionCollider = false;
		bool m_ShowAllColliders = false;
	
		friend class Scene;
		friend class EditorLayer;
		friend class SettingsPanel;
		friend class EditorMenuBar;
	};

}
#endif
