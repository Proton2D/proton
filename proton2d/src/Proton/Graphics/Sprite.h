#pragma once
#include "Proton/Graphics/Spritesheet.h"

namespace proton {

	class Sprite
	{
	public:
		Sprite() = default;
		Sprite(Shared<Texture> texture);
		Sprite(Shared<Spritesheet> spritesheet);

		const Shared<Texture>& GetTexture() const { return m_Texture; };

		void SetTextureFromPath(const std::string& filepath);
		void SetTexture(Shared<Texture> texture);
		void SetSpritesheet(Shared<Spritesheet> spritesheet);

		void SetTile(uint32_t x, uint32_t y);
		void SetTileX(uint32_t x);
		void SetTileY(uint32_t y);

		void SetTileSize(uint32_t tilesWidth, uint32_t tilesHeight);
		const glm::uvec2& GetTileSize() const { return m_TileSize; }
		const glm::uvec2& GetTilePos() const { return m_TilePos; }
		const glm::uvec2& GetPixelSize() const { return m_PixelSize; }
		float GetAspectRatio() const { return (float)m_PixelSize.x / (float)m_PixelSize.y; }

		void MirrorFlip(bool mirror_x, bool mirror_y);

		operator bool() const { return m_Texture != nullptr; }

	private:
		void CalculateTextureCoords();
		const TextureCoords& GetTextureCoords() const;
		
	private:
		Shared<Texture> m_Texture = nullptr;
		Shared<Spritesheet> m_Spritesheet = nullptr;

		TextureCoords m_TextureCoords = { {{ 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }} };
		
		glm::uvec2 m_PixelSize = { 0, 0 };
		glm::uvec2 m_TilePos   = { 0, 0 };
		glm::uvec2 m_TileSize  = { 1, 1 };
		bool m_MirrorFlipX = false, m_MirrorFlipY = false;

		friend class Renderer;
		friend class Scene;
		friend class Entity;
		friend class AssetManager;

		friend class InspectorPanel;
		friend class SceneSerializer;
	};

}
