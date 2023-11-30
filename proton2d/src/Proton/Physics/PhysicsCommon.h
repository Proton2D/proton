#pragma once
#include "Proton/Core/UUID.h"

#include <functional>
#include <box2d/b2_contact.h>
#include <box2d/b2_world_callbacks.h>

namespace proton {

	class Scene;

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
	};

	struct PhysicsContactCallback
	{
		std::function<void(PhysicsContact contact)> 
			OnBeginContactFunction = nullptr;

		std::function<void(PhysicsContact contact)>
			OnEndContactFunction = nullptr;

		std::function<void(PhysicsContact contact, const b2Manifold* oldManifold)>
			OnPreSolveFunction = nullptr;

		std::function<void(PhysicsContact contact, const b2ContactImpulse* impulse)>
			OnPostSolveFunction = nullptr;
	};

}
