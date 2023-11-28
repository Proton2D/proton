#pragma once
#include "Proton/Core/UUID.h"

#include <functional>
#include <box2d/b2_contact.h>
#include <box2d/b2_world_callbacks.h>


namespace proton {

	struct PhysicsMaterial
	{
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;
		float Density = 0.5f;
	};

	struct PhysicsContactInfo
	{
		UUID OtherUUID;
		b2Contact* Contact;
	};

	struct PhysicsContactCallback
	{
		std::function<void(PhysicsContactInfo info)> 
			OnBeginContactFunction = nullptr;

		std::function<void(PhysicsContactInfo info)>
			OnEndContactFunction = nullptr;

		std::function<void(PhysicsContactInfo info, const b2Manifold* oldManifold)>
			OnPreSolveFunction = nullptr;

		std::function<void(PhysicsContactInfo info, const b2ContactImpulse* impulse)> 
			OnPostSolveFunction = nullptr;
	};

}
