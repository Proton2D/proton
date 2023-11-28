#include "ptpch.h"
#include "Proton/Assets/AssetManager.h"
#include "Proton/Utils/Utils.h"

namespace proton {

	AssetManager* AssetManager::s_Instance = nullptr;

	void AssetManager::Init()
	{
		if (!s_Instance)
		{
			s_Instance = new AssetManager();
			ReloadAssetsList();
		}
	}

	Shared<Texture> AssetManager::LoadTexture(const std::string& filepath, bool autoLoad)
	{
		auto texture = MakeShared<Texture>(filepath);
		if (!texture->IsLoaded()) 
		{
			PT_CORE_ERROR("[AssetManager::LoadTexture] Couldn't load texture '{}'", filepath);
			return nullptr;
		}

		PT_CORE_INFO("[AssetManager::LoadTexture] file='{}'", filepath);
		s_Instance->m_Textures[filepath] = texture;
		return texture;
	}

	Shared<Spritesheet> AssetManager::LoadSpritesheet(const std::string& filepath)
	{
		auto texture = GetTexture(filepath);
		if (!texture)
			return nullptr;
		
		auto& spritesheetList = s_Instance->m_SpritesheetList;
		if (spritesheetList.find(filepath) == spritesheetList.end())
		{
			PT_CORE_ERROR("[AssetManager::LoadSpritesheet] Spritesheet not found in 'content/spritesheet.json'");
			return nullptr;
		}

		const auto& size = spritesheetList.at(filepath);
		PT_CORE_INFO("[AssetManager::LoadSpritesheet] file='{}' tile_size=({},{})", filepath, size.x, size.y);
		return MakeShared<Spritesheet>(texture, size.x, size.y);
	}

	bool AssetManager::IsTextureLoaded(const std::string& filepath)
	{
		return s_Instance->m_Textures.find(filepath) != s_Instance->m_Textures.end();
	}

	bool AssetManager::IsSpritesheetLoaded(const std::string& filepath)
	{
		return s_Instance->m_Spritesheets.find(filepath) != s_Instance->m_Spritesheets.end();
	}

	Shared<Texture> AssetManager::GetTexture(const std::string& filepath)
	{
		if (!IsTextureLoaded(filepath))
		{
			if (LoadTexture(filepath))
				return s_Instance->m_Textures[filepath];

			PT_CORE_ERROR("[AssetManager::GetTexture] Texture not loaded '{}'", filepath);
			return nullptr;
		}

		return s_Instance->m_Textures.at(filepath);
	}

	Shared<Spritesheet> AssetManager::GetSpritesheet(const std::string& filepath)
	{
		if (!IsSpritesheetLoaded(filepath))
		{
			auto spritesheet = LoadSpritesheet(filepath);
			if (!spritesheet)
			{
				PT_CORE_ERROR("[AssetManager::GetSpritesheet] Spritesheet not found '{}'", filepath);
				return nullptr;
			}
			s_Instance->m_Spritesheets[filepath] = spritesheet;
		}

		return s_Instance->m_Spritesheets.at(filepath);
	}

	bool AssetManager::UnloadTexture(const std::string& filepath)
	{
		if (!IsTextureLoaded(filepath))
			return false;

		s_Instance->m_Textures.erase(filepath);
		return true;
	}

	bool AssetManager::UnloadSpritesheet(const std::string& filepath)
	{
		if (!IsSpritesheetLoaded(filepath))
			return false;

		s_Instance->m_Spritesheets.erase(filepath);
		return true;
	}

	void AssetManager::ReloadAssetsList()
	{
		auto& textureList = s_Instance->m_TexturesFilepathList;
		auto& spritesheetList = s_Instance->m_SpritesheetList;

		textureList.clear();
		spritesheetList.clear();

		textureList = Utils::ScanDirectoryRecursive("content/textures",
			{ ".bmp", ".png", ".jpg", ".jpeg", ".tga", ".hdr", ".pic", ".psd" });

		for (auto& s : json::parse(Utils::ReadFile("content/spritesheet.json")))
		{
			std::string filepath = s["file_path"];
			uint32_t width = s["tile_width"], height = s["tile_height"];
			spritesheetList[filepath] = glm::uvec2{ width, height };
			textureList.erase(
				std::remove(textureList.begin(), textureList.end(), filepath),
				textureList.end()
			);
		}
	}
}
