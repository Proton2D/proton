#include "ptpch.h"
#include "Proton/Scene/Entity.h"
#include "Proton/Scene/EntityComponent.h"
#include "Proton/Scripting/EntityScript.h"
#include "Proton/Physics/PhysicsWorld.h"

#include <box2d/b2_body.h>

namespace proton 
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Scene(scene), m_Handle(handle) 
	{
	}

	void Entity::RemoveScript(const std::string& scriptClassName)
	{
		auto& component = GetComponent<ScriptComponent>();
		delete component.Scripts.at(scriptClassName);
		component.Scripts.erase(scriptClassName);
		if (!component.Scripts.size())
			RemoveComponent<ScriptComponent>();
	}

	bool Entity::IsValid()
	{
		if (m_Handle == entt::null)
			return false;

		if (!m_Scene->m_Registry.valid(m_Handle))
		{
			m_Handle = entt::null;
			return false;
		}
		return true;
	}

	Entity Entity::CreateChildEntity(const std::string& name) const
	{
		Entity entity = m_Scene->CreateEntity(name);
		AddChildEntity(entity, false);
		return entity;
	}

	void Entity::DestroyChildEntities() const
	{
		m_Scene->DestroyChildEntities(*this);
	}

	void Entity::AddChildEntity(Entity child, bool refreshChildWorldPosition) const
	{
		auto& parentComponent = GetComponent<RelationshipComponent>();
		auto& childComponent = child.GetComponent<RelationshipComponent>();

		childComponent.Parent = m_Handle;

		if (parentComponent.ChildrenCount)
		{
			Entity firstEntity = { parentComponent.First, m_Scene };
			firstEntity.GetComponent<RelationshipComponent>().Prev = child.m_Handle;
			childComponent.Next = parentComponent.First;
		}

		parentComponent.First = child.m_Handle;
		parentComponent.ChildrenCount++;

		std::vector<Entity>& sceneRoot = m_Scene->m_Root;
		auto it = std::find(sceneRoot.begin(), sceneRoot.end(), child);
		if (it != sceneRoot.end())
			sceneRoot.erase(it);

		if (refreshChildWorldPosition)
			child.SetWorldPosition(child.GetTransform().WorldPosition);
	}

	void Entity::PopHierarchy() const
	{
		auto& rc = GetComponent<RelationshipComponent>();
		if (rc.Parent != entt::null)
		{
			Entity parent{ rc.Parent, m_Scene };
			Entity prev{ rc.Prev, m_Scene };
			Entity next{ rc.Next, m_Scene };

			auto& parentRc = parent.GetComponent<RelationshipComponent>();
			parentRc.ChildrenCount--;
			if (parentRc.First == *this)
				parentRc.First = rc.Next;

			if (prev)
			{
				auto& prc = prev.GetComponent<RelationshipComponent>();
				prc.Next = rc.Next;
			}
			if (next)
			{
				auto& nrc = next.GetComponent<RelationshipComponent>();
				nrc.Prev = rc.Prev;
			}
			rc.Next = entt::null;
			rc.Prev = entt::null;
			rc.Parent = entt::null;

			m_Scene->m_Root.push_back(*this);
			auto& transform = GetTransform();
			transform.LocalPosition = transform.WorldPosition;
		}
	}

	bool Entity::IsParentOf(Entity entity) const
	{
		auto& rc = GetComponent<RelationshipComponent>();
		if (!rc.ChildrenCount)
			return false;

		Entity current{ rc.First, m_Scene };
		while (current)
		{
			if (current == entity)
				return true;

			auto& rc = current.GetComponent<RelationshipComponent>();
			if (rc.ChildrenCount && current.IsParentOf(entity))
				return true;

			current = Entity{ rc.Next, m_Scene };
		}

		return false;
	}

	void Entity::Destroy()
	{
		m_Scene->DestroyEntity(*this);
		m_Handle = entt::null;
	}

	Scene* Entity::GetScene() const
	{
		return m_Scene;
	}

	UUID Entity::GetUUID() const
	{
		return GetComponent<IDComponent>().ID;
	}

	const std::string& Entity::GetTag() const
	{
		return GetComponent<TagComponent>().Tag;
	}

	TransformComponent& Entity::GetTransform() const
	{
		return GetComponent<TransformComponent>();
	}

	Sprite& Entity::GetSprite() const
	{
		return GetComponent<SpriteComponent>().Sprite;
	}

	SpriteAnimation& Entity::GetSpriteAnimation() const
	{
		return GetComponent<SpriteAnimationComponent>().SpriteAnimation;
	}

	b2Body* Entity::GetRuntimeBody() const
	{
		auto& rb = GetComponent<RigidbodyComponent>();
		return rb.RuntimeBody;
	}

	void Entity::SetLinearVelocity(float x_mps, float y_mps) const
	{
		GetRuntimeBody()->SetLinearVelocity({x_mps, y_mps});
	}

	void Entity::SetLinearVelocityX(float mps) const
	{
		b2Body* body = GetRuntimeBody();
		body->SetLinearVelocity({ mps, body->GetLinearVelocity().y });
	}

	void Entity::SetLinearVelocityY(float mps) const
	{
		b2Body* body = GetRuntimeBody();
		body->SetLinearVelocity({ body->GetLinearVelocity().x, mps });
	}

	glm::vec2 Entity::GetLinearVelocity() const
	{
		b2Vec2 velocity = GetRuntimeBody()->GetLinearVelocity();
		if (glm::abs(velocity.x) < 0.00001f) velocity.x = 0.0f;
		if (glm::abs(velocity.y) < 0.00001f) velocity.y = 0.0f;
		return glm::vec2{ velocity.x, velocity.y };
	}

	void Entity::ApplyLinearImpulse(const glm::vec2& impulse, const glm::vec2& point) const
	{
		b2Body* body = GetRuntimeBody();
		if (point.x == 0.0f && point.y == 0.0f)
			body->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, true);
		else
			body->ApplyLinearImpulse({impulse.x, impulse.y }, {point.x, point.y}, true);
	}

	void Entity::SetWorldPosition(const glm::vec3& position) const
	{
		m_Scene->SetEntityWorldPosition(*this, position);
	}

	void Entity::SetLocalPosition(const glm::vec3& position) const
	{
		m_Scene->SetEntityLocalPosition(*this, position);
	}

	void Entity::SetRotationCenter(float angle) const
	{
		b2Body* body = GetRuntimeBody();
		if (body)
		{
			body->SetTransform(body->GetPosition(), angle * b2_pi);
			return;
		}
		GetTransform().Rotation = angle;
	}

	void Entity::RotateCenter(float angle) const
	{
		b2Body* body = GetRuntimeBody();
		if (body)
		{
			body->SetTransform(body->GetPosition(), body->GetAngle() + angle * b2_pi);
			return;
		}
		GetTransform().Rotation += angle;
	}

	void Entity::DestroyAllScripts()
	{
		auto& component = GetComponent<ScriptComponent>();
		for (auto& [scriptName, scriptInstance] : component.Scripts)
		{
			if (m_Scene->m_SceneState != SceneState::Stop)
				scriptInstance->OnDestroy();

			delete scriptInstance;
		}
		component.Scripts.clear();
	}

}
