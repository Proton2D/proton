#pragma once

namespace proton {

	class ApplicationConfig
	{
	public:
		ApplicationConfig();
		
		std::string WindowTitle = std::string();
		int WindowWidth = 1280;
		int WindowHeight = 720;
		bool Fullscreen = false;
		bool VSync = true;

		void LoadConfig();
		void WriteConfig();

	private:
		std::string m_Filepath = "app-config.json";
	};

#ifdef PT_EDITOR
	class EditorConfig
	{
	public:
		EditorConfig();

		struct Font
		{
			std::string FontFilepath;
			float  FontSize;
		};
		std::map<std::string, Font> EditorFonts;

		void LoadConfig();
		void WriteConfig();

	private:
		std::string m_Filepath = "editor/editor-config.json";
	};
#endif

}

