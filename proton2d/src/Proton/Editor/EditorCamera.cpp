#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/EditorCamera.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Core/Application.h"
#include "Proton/Core/Input.h"
#include "Proton/Events/MouseEvents.h"
#include "Proton/Events/WindowEvents.h"

#include <imgui.h>

namespace proton {

	EditorCamera::EditorCamera(const Shared<Camera>& camera)
		: m_AspectRatio(Application::Get().GetWindow().GetAspectRatio()),
		m_Camera(camera), m_Position({ 0.0f, 0.0f, 0.0f })
	{
	}

	void EditorCamera::OnUpdate(float ts)
	{
		Scene* activeScene = EditorLayer::Get()->m_ActiveScene;
		if (!activeScene)
			return;

		if (activeScene->m_SceneState == SceneState::Stop || EditorLayer::Get()->m_UseEditorCameraInRuntime)
		{
			float zoomLevel = m_Camera->GetZoomLevel();
			float zoomTargetDiff = glm::abs(m_ZoomLevelTarget - zoomLevel);
			float zoomOffset = glm::max(glm::round(zoomTargetDiff * ts * 10000.0f) / 1000.0f, 0.001f);

			if (m_ZoomLevelTarget > zoomLevel)
				m_Camera->SetZoomLevel(glm::min(zoomLevel + zoomOffset, m_ZoomLevelTarget));

			else if (m_ZoomLevelTarget < zoomLevel)
				m_Camera->SetZoomLevel(glm::max(zoomLevel - zoomOffset, m_ZoomLevelTarget));
		}
	}	

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		Scene* activeScene = EditorLayer::Get()->m_ActiveScene;
		if (!activeScene) 
			return;

		if (activeScene->m_SceneState == SceneState::Stop || EditorLayer::Get()->m_UseEditorCameraInRuntime)
		{
			dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& event) -> bool 
			{
				float zoomOffset = m_CameraZoomSpeed * -event.GetYOffset();
				m_ZoomLevelTarget += round(zoomOffset * round(m_ZoomLevelTarget * 10.0f) * 1000.0f) / 10000.0f;
				m_ZoomLevelTarget = glm::min(glm::max(m_ZoomLevelTarget, 0.2f), 30.0f);
				return false;
			});
		}
	}

	void EditorCamera::OnViewportResize(float w, float h)
	{
		m_AspectRatio = w / h;
		m_Camera->SetAspectRatio(w / h);
	}

}
#endif // PT_EDITOR
