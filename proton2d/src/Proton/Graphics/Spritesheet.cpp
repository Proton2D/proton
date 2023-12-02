#include "ptpch.h"
#include "Proton/Graphics/Spritesheet.h"

namespace proton {

	Spritesheet::Spritesheet(Shared<Texture> texture, uint32_t tileWidth, uint32_t tileHeight)
		: m_SheetSize({ texture->GetWidth(), texture->GetHeight() }),
		m_TileSize(tileWidth, tileHeight), m_Texture(texture)
	{
		m_TileCount = m_SheetSize / m_TileSize;
		m_TileScale = glm::vec2{ 1.0f } / (glm::vec2)m_TileCount;

		uint32_t x = 0, y = 0;
		m_TextureCoords.resize(m_TileCount.x);
		for (auto& column : m_TextureCoords)
		{
			column.resize(m_TileCount.y);
			for (auto& tileTextureCoords : column)
			{
				// Texture coords: [0.0 - 1.0] range
				tileTextureCoords[0] = { x * m_TileScale.x, y * m_TileScale.y };
				tileTextureCoords[1] = { (x + 1) * m_TileScale.x, y * m_TileScale.y };
				tileTextureCoords[2] = { (x + 1) * m_TileScale.x, (y + 1) * m_TileScale.y };
				tileTextureCoords[3] = { x * m_TileScale.x, (y + 1) * m_TileScale.y };
				y++;
			}
			y = 0; x++;
		}
	}

	Shared<Texture> Spritesheet::GetTexture()
	{
		return m_Texture;
	}

	const TextureCoords& Spritesheet::GetTextureCoords(uint32_t x, uint32_t y) const
	{
		PT_CORE_ASSERT(x < m_TileCount.x && y < m_TileCount.y, "Tile position out of bounds!");
		return m_TextureCoords[x % m_TileCount.x][y % m_TileCount.y];
	}

}
