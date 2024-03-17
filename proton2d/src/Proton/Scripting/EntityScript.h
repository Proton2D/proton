#pragma once
#include "Proton/Scene/Entity.h"
#include "Proton/Scripting/ScriptFactory.h"

// Script class must inherit from EntityScript class.
// This macro registers script class inside engine ScriptFactory.
// It helps engine know all the scripts you created and attach
// those scripts to entities by providing script class name as string.
#define ENTITY_SCRIPT_CLASS(script_class) \
static inline const char __ScriptClassName[] = #script_class; \
static inline const bool __RegisteredInFactory = \
	proton::ScriptFactory::Get().RegisterScript([&](proton::Entity entity) { \
		return entity.AddScript<script_class>(); \
	}, #script_class);

namespace proton {

	// Supported script variable types for Serialization / Editor view.
	enum class ScriptFieldType { Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, Bool };

	struct ScriptField
	{
		ScriptFieldType Type = ScriptFieldType::Float;
		void* InstanceFieldValue = nullptr;
		bool ShowInEditor = true;
	};

	// Base class for entity scripts.
	// - Use ENTITY_SCRIPT_CLASS in derived classes for registration.
	// - Implement OnCreate, OnDestroy, OnUpdate for entity behavior.
	// - Use OnRegisterFields to register fields (variables) for Serialization / Editor view.
	class EntityScript : public Entity
	{
	public:
		virtual ~EntityScript() = default;

		virtual bool OnCreate() { return true; }
		virtual void OnDestroy() {}
		virtual void OnUpdate(float ts) {}

		// Register your fields (variables) here. Use RegisterField function.
		// Supported variable types are listed inside ScriptFieldType enum.
		virtual void OnRegisterFields() {}

		// Draw your custom ImGui Editor interface here.
		// The ImGui elements will be displayed within the script node in the Inspector Panel.
		// IMPORTANT: Ensure that your ImGui code is enclosed within #ifdef PT_EDITOR preprocessor directives.
		virtual void OnImGuiRender() {}

		// Use glm::value_ptr for FloatX and IntX field types.
		// Supported variable types are listed inside ScriptFieldType enum.
		void RegisterField(ScriptFieldType type, const std::string& name, void* field, bool showInEditor = true);

	private:
		void SetFieldValueData(const std::string& fieldName, void* value);
		
	private:
		bool m_Stopped = false;
		bool m_Initialized = false;

		std::map<std::string, ScriptField> m_ScriptFields;

		friend class Entity;
		friend class Scene;
		friend class SceneSerializer;

		friend class InspectorPanel;
	};
}
