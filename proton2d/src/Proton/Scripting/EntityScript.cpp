#include "ptpch.h"
#include "Proton/Scripting/EntityScript.h"

namespace proton {

	void EntityScript::RegisterField(ScriptFieldType type, const std::string& name, void* field, bool showInEditor)
	{
		m_ScriptFields[name] = { type, field, showInEditor };
	}

	void EntityScript::SetFieldValueData(const std::string& fieldName, void* valuePtr)
	{
		ScriptField& field = m_ScriptFields[fieldName];

		switch (field.Type)
		{
		case ScriptFieldType::Float:
			*(float*)field.InstanceFieldValue = *(float*)valuePtr;
			break;
		case ScriptFieldType::Float2:
			*(glm::vec2*)field.InstanceFieldValue = *(glm::vec2*)valuePtr;
			break;
		case ScriptFieldType::Float3:
			*(glm::vec3*)field.InstanceFieldValue = *(glm::vec3*)valuePtr;
			break;
		case ScriptFieldType::Float4:
			*(glm::vec4*)field.InstanceFieldValue = *(glm::vec4*)valuePtr;
			break;
		case ScriptFieldType::Int:
			*(int*)field.InstanceFieldValue = *(int*)valuePtr;
			break;
		case ScriptFieldType::Int2:
			*(glm::ivec2*)field.InstanceFieldValue = *(glm::ivec2*)valuePtr;
			break;
		case ScriptFieldType::Int3:
			*(glm::ivec3*)field.InstanceFieldValue = *(glm::ivec3*)valuePtr;
			break;
		case ScriptFieldType::Int4:
			*(glm::ivec4*)field.InstanceFieldValue = *(glm::ivec4*)valuePtr;
			break;
		case ScriptFieldType::Bool:
			*(bool*)field.InstanceFieldValue = *(bool*)valuePtr;
			break;
		default:
			PT_ASSERT("Unexpected value type!");
			break;
		}
	}

}
