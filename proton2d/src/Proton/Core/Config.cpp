#include "ptpch.h"
#include "Proton/Core/Config.h"
#include "Proton/Utils/Utils.h"

#include <nlohmann/json.hpp>

namespace proton {

	using json = nlohmann::json;

	ApplicationConfig::ApplicationConfig()
	{
		LoadConfig();
	}

	void ApplicationConfig::LoadConfig()
	{
		if (!std::filesystem::exists(m_Filepath))
		{
			PT_CORE_WARN("'{}' file not found! Creating with default values.", m_Filepath);
			WindowTitle = "Proton2D Engine";
			WriteConfig();
			return;
		}
		
		json jsonObj = jsonObj.parse(Utils::ReadFile(m_Filepath));
		WindowTitle = jsonObj["window_title"];
		WindowWidth = jsonObj["window_width"];
		WindowHeight = jsonObj["window_height"];
		Fullscreen = jsonObj["fullscreen"];
		VSync = jsonObj["vsync"];
		
	}

	void ApplicationConfig::WriteConfig()
	{
		json jsonObj;
		jsonObj["window_title"] = WindowTitle;;
		jsonObj["window_width"] = WindowWidth;
		jsonObj["window_height"] = WindowHeight;
		jsonObj["fullscreen"] = Fullscreen;
		jsonObj["vsync"] = VSync;
		std::ofstream configFile(m_Filepath);
		configFile << jsonObj.dump(4);
		configFile.close();
	}

#ifdef PT_EDITOR
	EditorConfig::EditorConfig()
	{
		LoadConfig();
	}

	void EditorConfig::LoadConfig()
	{
		EditorFonts.clear();
		
		if (!std::filesystem::exists(m_Filepath))
		{
			PT_CORE_WARN("'{}' file not found! Creating config with default values.", m_Filepath);
			EditorFonts.insert(std::pair("roboto", Font{ "editor/content/font/Roboto.ttf", 18.0f }));
			WriteConfig();
			return;
		}

		json jsonObj = jsonObj.parse(Utils::ReadFile(m_Filepath));
		for (const auto& font : jsonObj["fonts"].items())
			EditorFonts.insert(std::pair(font.key(), Font{font.value()["font_filepath"], font.value()["font_size"]}));
	}

	void EditorConfig::WriteConfig()
	{
		json jsonObj;
		for (const auto& font : EditorFonts)
		{
			jsonObj["fonts"][font.first] = {
				{ "font_filepath", font.second.FontFilepath },
				{ "font_size", font.second.FontSize }
			};
		}
		std::ofstream configFile(m_Filepath);
		configFile << jsonObj.dump(4);
		configFile.close();
	}
#endif

}
