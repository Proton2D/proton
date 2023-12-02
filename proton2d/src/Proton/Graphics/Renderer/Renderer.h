// 
// Basic 2D Batch Renderer
// Architecture and Code based upon Hazel 2D Renderer:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Renderer/Renderer2D.h
// 
#pragma once

#define DEFAULT_CLEAR_COLOR { 0.1f, 0.12f, 0.16f, 1.0f }

#include "Proton/Graphics/Sprite.h"
#include "Proton/Graphics/Camera.h"

namespace proton {

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::vec3& position);
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, float tilingFactor = 1.0f);
		static void DrawQuad(const glm::mat4& transform, const Sprite& sprite, const glm::vec4& tintColor = glm::vec4(1.0f), float tilingFactor = 1.0f);
		static void DrawQuad(const glm::mat4& transform, const Shared<Texture>& texture, const TextureCoords& textureCoords, const glm::vec4& tintColor, float tilingFactor = 1.0f);

		static void DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color);
		static void DrawDashedLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, float lineScale = 1.0f);
		static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& color);
		static void DrawDashedRect(const glm::mat4& transform, const glm::vec4& color, float lineScale = 1.0f);

		static void SetLineWidth(float width);
		static void SetClearColor(glm::vec4 color);
		static void Clear();
		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		static void SetMaxQuadsCount(uint32_t count);
		static uint32_t GetDrawCallsCount();
		
	private:
		static void StartBatch();
		static void NextBatch();
	};

}
