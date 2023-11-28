#include "Proton/Graphics/Sprite.h"

#include <unordered_map>
#include <nlohmann/json.hpp>

namespace proton {

	using json = nlohmann::ordered_json;

	class AssetManager
	{
	public:
		static void Init();

		// Load texture and store using filepath as key.
		static Shared<Texture> LoadTexture(const std::string& filepath, bool autoLoad = true);

		// Get OpenGL Texture object pointer.
		static Shared<Texture> GetTexture(const std::string& filepath);

		// Delete OpenGL Texture object and free up memory.
		static bool UnloadTexture(const std::string& filepath);

		// Check if OpenGL Texture object is loaded in memory.
		static bool IsTextureLoaded(const std::string& filepath);

		// Load spritesheet and store using filepath as key.
		static Shared<Spritesheet> LoadSpritesheet(const std::string& filepath);

		// Returns Spritesheet object pointer.
		static Shared<Spritesheet> GetSpritesheet(const std::string& filepath);

		// Delete Spritesheet object and free up memory.
		static bool UnloadSpritesheet(const std::string& filepath);

		// Check if Spritesheet object is loaded in memory.
		static bool IsSpritesheetLoaded(const std::string& filepath);

		// Reload list of assets in "assets" directory.
		// Reload Spritesheet list from "spritesheets.json" file.
		static void ReloadAssetsList();

	private:
		static AssetManager* s_Instance;

		std::unordered_map<std::string, Shared<Texture>> m_Textures;
		std::unordered_map<std::string, Shared<Spritesheet>> m_Spritesheets;

		std::vector<std::string> m_TexturesFilepathList;
		std::unordered_map<std::string, glm::uvec2> m_SpritesheetList;

		friend class InspectorPanel;
	};

}
