#pragma once

namespace proton {

	class Project
	{
	public:
		bool LoadProjectSettings();
		void WriteProjectSettings();

	private:
		std::string m_StartScene;
		const std::string m_Filepath = "content/project.json";

		friend class Application;
		friend class SettingsPanel;
	};

}
