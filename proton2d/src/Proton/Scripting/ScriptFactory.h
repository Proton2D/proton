#pragma once
#include "Proton/Scene/Entity.h"

namespace proton {

	class EntityScript;
	using AddScriptFunction = std::function<EntityScript*(Entity entity)>;

	class ScriptFactory
	{
	public:
		static ScriptFactory& Get(); // Get singleton instance

		EntityScript* AddScriptToEntity(Entity entity, const std::string& className);
		bool RegisterScript(const AddScriptFunction& addFunction, const std::string& className);

	private:
		std::unordered_map<std::string, AddScriptFunction> m_ScriptRegistry;

		friend class InspectorPanel;
	};
}
