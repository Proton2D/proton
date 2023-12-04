#include "ptpch.h"
#include "Proton/Scripting/ScriptFactory.h"

namespace proton {

	ScriptFactory& ScriptFactory::Get()
	{
		static ScriptFactory instance;
		return instance;
	}

	EntityScript* ScriptFactory::AddScriptToEntity(Entity entity, const std::string& className)
	{
		if (m_ScriptRegistry.find(className) == m_ScriptRegistry.end())
		{
			PT_CORE_ERROR("[ScriptFactory::AddScriptToEntity] Script '{}' not found!", className);
			return nullptr;
		}
		AddScriptFunction& addScriptFunction = m_ScriptRegistry.at(className);
		EntityScript* script = addScriptFunction(entity);
		return script;
	}

	bool ScriptFactory::RegisterScript(const AddScriptFunction& addFunction, const std::string& className)
	{
		if (m_ScriptRegistry.find(className) != m_ScriptRegistry.end())
		{
			PT_CORE_ERROR("Script registration failed, script '{}' already exists", className);
			return false;
		}
		m_ScriptRegistry[className] = addFunction;
		return true;
	}

}
