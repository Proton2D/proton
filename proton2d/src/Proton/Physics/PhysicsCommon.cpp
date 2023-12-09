#include "ptpch.h"
#include "Proton/Physics/PhysicsCommon.h"
#include "Proton/Scene/Entity.h"
#include "Proton/Scripting/EntityScript.h"

namespace proton {

	bool PhysicsContact::OtherCompare(Entity* entity)
	{
		return *Other == *entity;
	}

	bool PhysicsContact::OtherCompare(EntityScript* script)
	{
		return Other->m_Handle == script->m_Handle;
	}

}
