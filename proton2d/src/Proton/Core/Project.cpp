#include "ptpch.h"
#include "Proton/Core/Project.h"
#include "Proton/Utils/Utils.h"

#include <nlohmann/json.hpp>

namespace proton {

	using json = nlohmann::json;

	bool Project::LoadProjectSettings()
	{
		if (!std::filesystem::exists(m_Filepath))
		{
			PT_CORE_ERROR("'{}' file not found!", m_Filepath);
			return false;
		}

		json jsonObj = jsonObj.parse(Utils::ReadFile(m_Filepath));
		if (!jsonObj.contains("start_scene"))
		{
			PT_CORE_ERROR("'start_scene' missing in '{}'!", m_Filepath);
			return false;
		}

		m_StartScene = jsonObj.at("start_scene");

		return true;
	}

	void Project::WriteProjectSettings()
	{
		json jsonObj;
		jsonObj["start_scene"] = m_StartScene;
		std::ofstream configFile(m_Filepath);
		configFile << jsonObj.dump(4);
		configFile.close();
	}

}
