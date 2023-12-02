#include "ptpch.h"
#include "Proton/Scene/Entity.h"
#include "Proton/Scripting/EntityScript.h"

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

	TransformComponent& Entity::GetTransform()
	{
		return GetComponent<TransformComponent>();
	}

	UUID Entity::GetUUID() const
	{
		return GetComponent<IDComponent>().ID;
	}

	const std::string& Entity::GetTag() const
	{
		return GetComponent<TagComponent>().Tag;
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

	b2Body* Entity::GetRuntimeBody()
	{
		auto& id = GetComponent<IDComponent>();
		return m_Scene->GetRuntimeBody(id.ID);
	}

	void Entity::SetVelocity(float x_mps, float y_mps)
	{
		b2Body* body = GetRuntimeBody();
		body->SetLinearVelocity({ x_mps, y_mps });
	}

	void Entity::SetVelocityX(float mps)
	{
		b2Body* body = GetRuntimeBody();
		body->SetLinearVelocity({ mps, body->GetLinearVelocity().y });
	}

	void Entity::SetVelocityY(float mps)
	{
		b2Body* body = GetRuntimeBody();
		body->SetLinearVelocity({ body->GetLinearVelocity().x, mps });
	}

	glm::vec2 Entity::GetVelocity()
	{
		b2Vec2 velocity = GetRuntimeBody()->GetLinearVelocity();
		return glm::vec2{ velocity.x, velocity.y };
	}

	void Entity::ApplyImpulse(const glm::vec2& impulse)
	{
		b2Body* body = GetRuntimeBody();
		body->ApplyLinearImpulse({impulse.x, impulse.y }, body->GetWorldCenter(), true);
	}

	void Entity::TerminateScripts()
	{
		for (auto& [scriptName, scriptInstance] : GetComponent<ScriptComponent>().Scripts)
		{
			if (m_Scene->m_SceneState != SceneState::Stop)
				scriptInstance->OnDestroy();

			delete scriptInstance;
			scriptInstance = nullptr;
		}
	}

	void Entity::Destroy()
	{
		m_Scene->DestroyEntity(*this);
		m_Handle = entt::null;
	}

	void Entity::DestroyChildEntities()
	{
		m_Scene->DestroyChildEntities(*this);
	}

	void Entity::AddChildEntity(Entity child, bool refreshChildWorldPosition)
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

	void Entity::PopHierarchy()
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

	bool Entity::IsParentOf(Entity entity)
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

	void Entity::SetWorldPosition(const glm::vec3& position)
	{
		m_Scene->SetEntityWorldPosition(*this, position);
	}

	void Entity::SetLocalPosition(const glm::vec3& position)
	{
		m_Scene->SetEntityLocalPosition(*this, position);
	}

}
