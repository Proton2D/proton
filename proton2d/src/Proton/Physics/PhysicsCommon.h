#pragma once
#include "Proton/Core/UUID.h"

#include <functional>
#include <box2d/b2_contact.h>
#include <box2d/b2_world_callbacks.h>

namespace proton {

	// Forward declaration
	class Entity;
	class EntityScript;

	struct PhysicsMaterial
	{
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;
		float Density = 0.5f;
	};

	struct PhysicsContact
	{
		Entity* Other;
		b2Contact* Contact;

		bool OtherCompare(Entity* entity);

		bool OtherCompare(EntityScript* script);
	};

	struct PhysicsContactCallback
	{
		uint32_t ContactCount = 0;

		std::function<void(PhysicsContact contact)> 
			OnBegin = nullptr;

		std::function<void(PhysicsContact contact)>
			OnEnd = nullptr;

		std::function<void(PhysicsContact contact, const b2Manifold* oldManifold)>
			OnPreSolve = nullptr;

		std::function<void(PhysicsContact contact, const b2ContactImpulse* impulse)>
			OnPostSolve = nullptr;
	};

}
