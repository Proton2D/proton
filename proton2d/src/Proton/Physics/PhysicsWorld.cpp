#include "ptpch.h"
#include "Proton/Physics/PhysicsWorld.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include "box2d/b2_circle_shape.h"
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
		PT_CORE_ASSERT(m_RuntimeBodies.find(id) != m_RuntimeBodies.end(), "Physics runtime body not found!");
		return m_RuntimeBodies.at(id);
	}

	void PhysicsWorld::DestroyRuntimeBody(UUID id)
	{
		m_World->DestroyBody(GetRuntimeBody(id));
		m_RuntimeBodies.erase(id);
	}

	b2Body* PhysicsWorld::CreateRuntimeBody(Entity entity)
	{
		auto& uuid = entity.GetComponent<IDComponent>().ID;
		PT_CORE_ASSERT(m_RuntimeBodies.find(uuid) == m_RuntimeBodies.end(), "Physics runtime body already exists!");

		auto& transform = entity.GetComponent<TransformComponent>();
		auto& rb = entity.GetComponent<RigidbodyComponent>();

		b2BodyDef bodyDef;
		bodyDef.type = rb.Type;
		bodyDef.position.Set(transform.WorldPosition.x, transform.WorldPosition.y);
		bodyDef.angle = glm::radians(transform.Rotation);

		b2Body* body = m_World->CreateBody(&bodyDef);
		body->SetFixedRotation(rb.FixedRotation);
		AddFixtureToRuntimeBody(entity, body);
		m_RuntimeBodies[entity.GetUUID()] = body;
		rb.RuntimeBody = body;
		
		return body;
	}

	void PhysicsWorld::AddFixtureToRuntimeBody(Entity entity, b2Body* body)
	{
		if (!entity.HasAnyComponent<BoxColliderComponent, CircleColliderComponent>())
			return;

		auto& transform = entity.GetComponent<TransformComponent>();
		auto& uuid = entity.GetComponent<IDComponent>().ID;

		b2FixtureDef fixtureDef;

		// Store Entity in fixture data (for collisions and contacts)
		m_FixtureUserData.push_back(MakeUnique<Entity>(entity.m_Handle, m_Scene));
		fixtureDef.userData.pointer = (uintptr_t)(m_FixtureUserData.back().get());

		if (!body)
			body = GetRuntimeBody(entity.GetUUID());

		// BoxColliderComponent
		if (entity.HasComponent<BoxColliderComponent>())
		{
			auto& bc = entity.GetComponent<BoxColliderComponent>();

			b2PolygonShape shape;
			shape.SetAsBox(bc.Size.x * transform.Scale.x / 2.0f,
				bc.Size.y * transform.Scale.y / 2.0f, { bc.Offset.x, bc.Offset.y }, 0);

			fixtureDef.shape = &shape;
			fixtureDef.isSensor = bc.IsSensor;
			fixtureDef.filter = bc.Filter;

			fixtureDef.friction = bc.Material.Friction;
			fixtureDef.restitution = bc.Material.Restitution;
			fixtureDef.restitutionThreshold = bc.Material.RestitutionThreshold;
			fixtureDef.density = bc.Material.Density;

			body->CreateFixture(&fixtureDef);
		}

		// CircleColliderComponent
		if (entity.HasComponent<CircleColliderComponent>())
		{
			auto& cc = entity.GetComponent<CircleColliderComponent>();

			b2CircleShape shape;
			shape.m_p.Set(cc.Offset.x, cc.Offset.y);
			shape.m_radius = transform.Scale.x / 2.0f * cc.Radius;
			fixtureDef.shape = &shape;
			fixtureDef.isSensor = cc.IsSensor;

			fixtureDef.friction = cc.Material.Friction;
			fixtureDef.restitution = cc.Material.Restitution;
			fixtureDef.restitutionThreshold = cc.Material.RestitutionThreshold;
			fixtureDef.density = cc.Material.Density;

			body->CreateFixture(&fixtureDef);
		}
	}

	void PhysicsWorld::BuildWorld()
	{
		// Initialize Box2D world
		m_World = new b2World({ 0.0f, -m_Gravity });
		m_World->SetContactListener((b2ContactListener*)&m_ContactListener);
		for (entt::entity entity : m_Scene->m_Registry.view<RigidbodyComponent>())
			CreateRuntimeBody(Entity{ entity, m_Scene });

		// Attach a fixture to the parent entity for each child which has
		// a BoxColliderComponent but does not have a RigidbodyComponent.
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
					AddFixtureToRuntimeBody(entity, body);
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
		PROFILE_FUNCTION();
		// Initialize entities created during game runtime
		if (m_EntitiesToInitialize.size())
		{
			auto& vec = m_EntitiesToInitialize;
			vec.erase(std::unique(vec.begin(), vec.end()), vec.end());

			for (auto& entity : m_EntitiesToInitialize)
			{
				if (entity.HasComponent<RigidbodyComponent>())
				{
					CreateRuntimeBody(entity);
					continue;
				}
				auto& rc = entity.GetComponent<RelationshipComponent>();
				if (rc.Parent != entt::null)
				{
					Entity parent{ rc.Parent, m_Scene };
					AddFixtureToRuntimeBody(entity, GetRuntimeBody(parent.GetUUID()));
				}
			}
			m_EntitiesToInitialize.clear();
		}

		// Update physics world
		m_World->Step(ts, m_PhysicsVelocityIterations, m_PhysicsPositionIterations);
		auto view = m_Scene->m_Registry.view<IDComponent, TransformComponent, RigidbodyComponent>();
		for (auto entity : view)
		{
			auto [id, transform] = view.get<IDComponent, TransformComponent>(entity);
			b2Body* body = m_RuntimeBodies.at(id.ID);
			// Retrive positions of entities
			transform.WorldPosition.x = body->GetPosition().x;
			transform.WorldPosition.y = body->GetPosition().y;
			transform.Rotation = glm::degrees(body->GetAngle());
		}
	}

	PhysicsContactListener::PhysicsContactListener(Scene* scene)
		: m_Scene(scene)
	{
	}

#define CALL_CONTACT_CALLBACK_FUNCTION(callback_func, ...) \
	Entity* entityA = (Entity*)(contact->GetFixtureA()->GetUserData().pointer); \
	Entity* entityB = (Entity*)(contact->GetFixtureB()->GetUserData().pointer); \
	if (!entityA->IsValid() || !entityB->IsValid()) return; \
	PhysicsContactCallback* callbackA = nullptr; \
	PhysicsContactCallback* callbackB = nullptr; \
\
	if (entityA->HasComponent<BoxColliderComponent>()) \
		callbackA = &entityA->GetComponent<BoxColliderComponent>().ContactCallback; \
	if (entityB->HasComponent<BoxColliderComponent>()) \
		callbackB = &entityB->GetComponent<BoxColliderComponent>().ContactCallback; \
\
	if (entityA->HasComponent<CircleColliderComponent>()) \
		callbackA = &entityA->GetComponent<CircleColliderComponent>().ContactCallback; \
	if (entityB->HasComponent<CircleColliderComponent>()) \
		callbackB = &entityB->GetComponent<CircleColliderComponent>().ContactCallback; \
\
	if (callbackA && callbackA->callback_func) \
		callbackA->callback_func(PhysicsContact{ entityB, contact }, __VA_ARGS__); \
	if (callbackB && callbackB->callback_func) \
		callbackB->callback_func(PhysicsContact{ entityA, contact }, __VA_ARGS__); \

	void PhysicsContactListener::BeginContact(b2Contact* contact)
	{
		CALL_CONTACT_CALLBACK_FUNCTION(OnBegin);
		if (callbackA)
			callbackA->ContactCount++;
		if (callbackB)
			callbackB->ContactCount++;
	}

	void PhysicsContactListener::EndContact(b2Contact* contact)
	{
		CALL_CONTACT_CALLBACK_FUNCTION(OnEnd);
		if (callbackA)
			callbackA->ContactCount--;
		if (callbackB)
			callbackB->ContactCount--;
	}

	void PhysicsContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		CALL_CONTACT_CALLBACK_FUNCTION(OnPreSolve, oldManifold);
	}

	void PhysicsContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		CALL_CONTACT_CALLBACK_FUNCTION(OnPostSolve, impulse);
	}

}
