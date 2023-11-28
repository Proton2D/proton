#pragma once
#ifdef PT_EDITOR

#include "Proton/Core/Base.h"
#include "Proton/Graphics/Camera.h"
#include "Proton/Events/Event.h"

namespace proton {

	class EditorCamera {
	public:
		EditorCamera(const Shared<Camera>& camera = MakeShared<Camera>());
		virtual ~EditorCamera() = default;

		void OnUpdate(float ts);
		void OnEvent(Event& e);

		Shared<Camera> GetBaseCamera() { return m_Camera; }
		const Shared<Camera>& GetBaseCamera() const { return m_Camera; }

		const glm::vec3& GetPosition() const { return m_Position; }

		void OnViewportResize(float w, float h);

	private:
		Shared<Camera> m_Camera;
		
		glm::vec3 m_Position;

		float m_AspectRatio = 16.0f / 9.0f;
		float m_CameraSpeed = 3.0f;
		float m_ZoomLevelTarget = 1.0f;
		float m_CameraZoomSpeed = 0.10f;

		friend class EditorLayer;
		friend class SceneSerializer;
	};
}
#endif // PT_EDITOR
