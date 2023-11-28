#include "ptpch.h"
#include "Proton/Physics/PhysicsWorld.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_contact.h>

namespace proton {

	PhysicsWorld::PhysicsWorld(Scene* context)
		: m_ContactListener(context), m_Scene(context)
	{
	}

	PhysicsWorld::~PhysicsWorld()
	{
		DestroyWorld();
	}

	b2Body* PhysicsWorld::GetRuntimeBody(UUID id)
	{
		PT_ASSERT(m_RuntimeBodies.find(id) != m_RuntimeBodies.end(), "Runtime body not found!");
		return m_RuntimeBodies.at(id);
	}

	void PhysicsWorld::DestroyRuntimeBody(UUID id)
	{
		PT_ASSERT(m_RuntimeBodies.find(id) != m_RuntimeBodies.end(), "Runtime body not found!");
		m_World->DestroyBody(m_RuntimeBodies.at(id));
		m_RuntimeBodies.erase(id);
	}

	b2Body* PhysicsWorld::CreateRuntimeBody(Entity entity)
	{
		auto& uuid = entity.GetComponent<IDComponent>().ID;
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& rb = entity.GetComponent<RigidbodyComponent>();

		b2BodyDef bodyDef;
		bodyDef.type = rb.Type;
		bodyDef.position.Set(transform.Position.x, transform.Position.y);
		bodyDef.angle = glm::radians(transform.Rotation);

		b2Body* body = m_World->CreateBody(&bodyDef);
		body->SetFixedRotation(rb.FixedRotation);
		AddFixtureRuntimeBody(body, entity);
		m_RuntimeBodies[entity.GetUUID()] = body;

		return body;
	}

	void PhysicsWorld::AddFixtureRuntimeBody(b2Body* body, Entity entity)
	{
		if (entity.HasComponent<BoxColliderComponent>())
		{
			auto& bc = entity.GetComponent<BoxColliderComponent>();
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& uuid = entity.GetComponent<IDComponent>().ID;

			b2FixtureDef fixtureDef; b2PolygonShape shape;
			m_FixtureUserData.push_back(MakeUnique<UUID>(uuid));

			shape.SetAsBox(bc.Size.x * transform.Scale.x / 2.0f,
				bc.Size.y * transform.Scale.y / 2.0f, { bc.Offset.x, bc.Offset.y }, 0);

			const PhysicsMaterial& material = bc.Material;
			fixtureDef.shape = &shape;
			fixtureDef.friction = material.Friction;
			fixtureDef.restitution = material.Restitution;
			fixtureDef.restitutionThreshold = material.RestitutionThreshold;
			fixtureDef.density = material.Density;
			fixtureDef.isSensor = bc.IsSensor;
			fixtureDef.filter = bc.Filter;
			fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(m_FixtureUserData.back().get());
			body->CreateFixture(&fixtureDef);
		}
	}

	void PhysicsWorld::BuildWorld()
	{
		// TODO: Change GetEntitiesWithComponents
		// Initialize b2World
		m_World = new b2World({ 0.0f, -m_Gravity });
		m_World->SetContactListener((b2ContactListener*)&m_ContactListener);
		for (entt::entity entity : m_Scene->m_Registry.view<RigidbodyComponent>())
			CreateRuntimeBody(Entity{ entity, m_Scene });

		// TODO: Think about it
		// Attach a fixture to the parent entity for each child entitiy
		// which has a BoxColliderComponent but lacks RigidbodyComponent.
		for (entt::entity e : m_Scene->m_Registry.view<BoxColliderComponent>(entt::exclude<RigidbodyComponent>))
		{
			Entity entity{ e, m_Scene };
			auto& rc = entity.GetComponent<RelationshipComponent>();
			if (rc.Parent != entt::null)
			{
				Entity parent{ rc.Parent, m_Scene };
				if (parent.HasComponent<RigidbodyComponent>())
				{
					b2Body* body = GetRuntimeBody(parent.GetUUID());
					AddFixtureRuntimeBody(body, entity);
				}
			}
		}
	}

	void PhysicsWorld::DestroyWorld()
	{
		if (m_World)
		{
			delete m_World;
			m_World = nullptr;
		}
		m_RuntimeBodies.clear();
		m_FixtureUserData.clear();
	}

	void PhysicsWorld::Update(float ts)
	{
		m_World->Step(ts, m_PhysicsVelocityIterations, m_PhysicsPositionIterations);
		auto view = m_Scene->m_Registry.view<IDComponent, TransformComponent, RigidbodyComponent>();
		for (auto entity : view)
		{
			auto [id, transform] = view.get<IDComponent, TransformComponent>(entity);
			b2Body* body = m_RuntimeBodies.at(id.ID);
			transform.Position.x = body->GetPosition().x;
			transform.Position.y = body->GetPosition().y;
			transform.Rotation = glm::degrees(body->GetAngle());
		}
	}

	PhysicsContactListener::PhysicsContactListener(Scene* scene)
		: m_Scene(scene)
	{
	}

#define GET_CONTACT_ENTITIES()\
	b2Fixture* fixtureA = contact->GetFixtureA();\
	b2Fixture* fixtureB = contact->GetFixtureB();\
	UUID uuidA = *(UUID*)(fixtureA->GetUserData().pointer);\
	UUID uuidB = *(UUID*)(fixtureB->GetUserData().pointer);\
	Entity entityA = m_Scene->FindByID(uuidA);\
	Entity entityB = m_Scene->FindByID(uuidB);\
	if (!entityA.IsValid() || !entityB.IsValid()) return; \
	auto& bcA = entityA.GetComponent<BoxColliderComponent>();\
	auto& bcB = entityB.GetComponent<BoxColliderComponent>();

	void PhysicsContactListener::BeginContact(b2Contact* contact)
	{
		GET_CONTACT_ENTITIES();

		if (bcA.ContactCallback.OnBeginContactFunction)
			bcA.ContactCallback.OnBeginContactFunction({ uuidB, contact });

		if (bcB.ContactCallback.OnBeginContactFunction)
			bcB.ContactCallback.OnBeginContactFunction({ uuidA, contact });
	}

	void PhysicsContactListener::EndContact(b2Contact* contact)
	{
		GET_CONTACT_ENTITIES();

		if (bcA.ContactCallback.OnEndContactFunction)
			bcA.ContactCallback.OnEndContactFunction({ uuidB, contact });

		if (bcB.ContactCallback.OnEndContactFunction)
			bcB.ContactCallback.OnEndContactFunction({ uuidA, contact });
	}

	void PhysicsContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		GET_CONTACT_ENTITIES();

		if (bcA.ContactCallback.OnPreSolveFunction)
			bcA.ContactCallback.OnPreSolveFunction({ uuidB, contact }, oldManifold);

		if (bcB.ContactCallback.OnPreSolveFunction)
			bcB.ContactCallback.OnPreSolveFunction({ uuidA, contact }, oldManifold);
	}

	void PhysicsContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		GET_CONTACT_ENTITIES();

		if (bcA.ContactCallback.OnPostSolveFunction)
			bcA.ContactCallback.OnPostSolveFunction({ uuidB, contact }, impulse);

		if (bcB.ContactCallback.OnPostSolveFunction)
			bcB.ContactCallback.OnPostSolveFunction({ uuidA, contact }, impulse);
	}

}
