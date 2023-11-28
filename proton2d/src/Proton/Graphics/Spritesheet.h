#pragma once
#include "Proton/Graphics/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace proton {

	// Source texture coordinates
	// [0] - bottom left (0,0)
	// [1] - bottom right (1,0)
	// [2] - top right (1,1)
	// [3] - top left (0,1)
	using TextureCoords = std::array<glm::vec2, 4>;

	class Spritesheet
	{
	public:
		Spritesheet(Shared<Texture> texture, uint32_t tileWidth, uint32_t tileHeight);

		// Returns pointer to OpenGL texture object
		Shared<Texture> GetTexture();

		// Get sheet size in pixels
		const glm::uvec2& GetSheetSize() const { return m_SheetSize; }
		// Get tile size in pixels
		const glm::uvec2& GetTileSize() const { return m_TileSize; }
		// Get max tiles count that can fit into spritesheet
		const glm::uvec2& GetTileCount() const { return m_TileCount; }

	private:
		const TextureCoords& GetTextureCoords(uint32_t x, uint32_t y) const;

		std::vector<std::vector<TextureCoords>> m_TextureCoords;
		Shared<Texture> m_Texture;

		glm::uvec2 m_SheetSize = { 0, 0 }; // pixels
		glm::uvec2 m_TileSize = { 0, 0 }; // pixels
		glm::uvec2 m_TileCount = { 0, 0 }; // sheet size / tile size
		glm::vec2 m_TileScale = { 0, 0 }; // 0.0f - 1.0f

		friend class InspectorPanel;
		friend class Sprite;
		friend class ResizableSprite;
		friend class Scene;
	};

}
