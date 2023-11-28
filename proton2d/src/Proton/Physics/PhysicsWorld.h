#pragma once
#include "Proton/Core/UUID.h"
#include "Proton/Scene/Entity.h"

class b2Body;

namespace proton {

	class Scene;

	class PhysicsContactListener : public b2ContactListener
	{
	public:
		PhysicsContactListener(Scene* scene);
		virtual void BeginContact(b2Contact* contact) override;
		virtual void EndContact(b2Contact* contact) override;
		virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
		virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

	private:
		Scene* m_Scene = nullptr;
	};

	class PhysicsWorld
	{
	public:
		PhysicsWorld(Scene* context);
		virtual ~PhysicsWorld();

		b2Body* GetRuntimeBody(UUID id);
		b2Body* CreateRuntimeBody(Entity entity);
		void DestroyRuntimeBody(UUID id);

		bool IsIntialized() const { return m_World; };
		operator bool() const { return m_World; }

	private:
		void BuildWorld();
		void DestroyWorld();
		void Update(float ts);

		void AddFixtureRuntimeBody(b2Body* body, Entity entity);
	
	private:
		b2World* m_World = nullptr;
		Scene* m_Scene = nullptr;

		std::unordered_map<UUID, b2Body*> m_RuntimeBodies;
		std::vector<Unique<UUID>> m_FixtureUserData;

		int m_PhysicsVelocityIterations = 5;
		int m_PhysicsPositionIterations = 5;
		float m_Gravity = 9.8f;
		
		PhysicsContactListener m_ContactListener;

		friend class Scene;
		friend class SceneSerializer;
		friend class InspectorPanel;
	};

}
