#pragma once

#include <glm/glm.hpp>

namespace proton {

	struct OrthoProjection
	{
		float Left, Right, Top, Bottom;
	};

	class Camera
	{
	public:
		Camera();
		virtual ~Camera() = default;

		void SetZoomLevel(float zoomLevel);
		void SetAspectRatio(float aspectRatio);
		float GetAspectRatio() const { return m_AspectRatio; }
		float GetZoomLevel() const { return m_ZoomLevel; }
		float GetOrthographicSize() const { return m_OrthographicSize; }

		const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
		const OrthoProjection& GetOrthoProjection() const { return m_Projection; }

		void RecalculateProjection();
	private:

	private:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;

		OrthoProjection m_Projection;
		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f;
		float m_OrthographicFar = 1.0f;
	};

}
