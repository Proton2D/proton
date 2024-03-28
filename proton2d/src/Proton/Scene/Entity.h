#pragma once

#include "Proton/Core/Base.h"
#include "Proton/Scene/Components.h"
#include "Proton/Scene/Scene.h"

namespace proton {

	// Entity class wrapper for the EnTT ECS
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);

		virtual ~Entity() = default;

		template <typename TComponent>
		TComponent& GetComponent() const
		{
			PT_CORE_ASSERT(HasComponent<TComponent>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<TComponent>(m_Handle);
		}

		template <typename TComponent, typename... TArgs>
		TComponent& AddComponent(TArgs&& ...args) const
		{
			PT_CORE_ASSERT(!HasComponent<TComponent>(), "Entity already have component!");
			return m_Scene->m_Registry.emplace<TComponent>(m_Handle, std::forward<TArgs>(args)...);
		}

		template<typename TComponent, typename... TArgs>
		TComponent& AddOrReplaceComponent(TArgs&&... args)
		{
			return m_Scene->m_Registry.emplace_or_replace<TComponent>(m_Handle, std::forward<Args>(args)...);
		}

		template <typename TComponent>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.any_of<TComponent>(m_Handle);
		}

		template <typename... TComponents>
		bool HasComponents() const
		{
			return m_Scene->m_Registry.all_of<TComponents...>(m_Handle);
		}

		template <typename... TComponents>
		bool HasAnyComponent() const
		{
			return m_Scene->m_Registry.any_of<TComponents...>(m_Handle);
		}

		// AddComponent overrides: Definitions in EntityComponent.h
		// TODO: Move these to Scene::OnComponentAdded<TComponent>
		template<> ResizableSpriteComponent& AddComponent() const;
		template<> RigidbodyComponent& AddComponent() const;
		template<> BoxColliderComponent& AddComponent() const;
		template<> SpriteAnimationComponent& AddComponent() const;

		template <typename TComponent>
		void RemoveComponent()
		{
			PT_CORE_ASSERT(HasComponent<TComponent>(), "Entity does not have a component!");

			if (std::is_base_of<ScriptComponent, TComponent>::value)
				DestroyAllScripts();

			if (std::is_base_of<CameraComponent, TComponent>::value
				&& m_Scene->m_PrimaryCameraEntity == *this)
			{
				PT_CORE_WARN("Scene Primary Camera has been removed!");
				m_Scene->m_PrimaryCameraEntity = entt::null;
				m_Scene->m_PrimaryCamera = nullptr;
			}

			m_Scene->m_Registry.remove<TComponent>(m_Handle);
		}

		template <typename TScriptClass>
		EntityScript* AddScript() const
		{
			if (!HasComponent<ScriptComponent>())
				AddComponent<ScriptComponent>();

			auto& component = GetComponent<ScriptComponent>();
			std::string className = TScriptClass::__ScriptClassName;
			PT_CORE_ASSERT(component.Scripts.find(className) == component.Scripts.end(), "The script is already attached to an Entity!");

			EntityScript*& scriptInstance = component.Scripts[className];
			scriptInstance = new TScriptClass();
			scriptInstance->m_Handle = m_Handle;
			scriptInstance->m_Scene = m_Scene;
			scriptInstance->OnRegisterFields();
			return scriptInstance;
		}

		void RemoveScript(const std::string& scriptClassName);
		
		// Entity lifetime
		bool IsValid();
		void Destroy();

		// Scene hierarchy
		Entity CreateChildEntity(const std::string& name) const;
		void AddChildEntity(Entity child, bool refreshChildWorldPosition = true) const;
		void DestroyChildEntities() const;
		void PopHierarchy() const;
		bool IsParentOf(Entity entity) const;

		// Component getters
		Scene* GetScene() const;
		UUID GetUUID() const;
		const std::string& GetTag() const;
		TransformComponent& GetTransform() const;
		Sprite& GetSprite() const;
		SpriteAnimation& GetSpriteAnimation() const;
		b2Body* GetRuntimeBody() const;

		// Transform modifiers
		void SetWorldPosition(const glm::vec3& position) const;
		void SetLocalPosition(const glm::vec3& position) const;
		void SetRotationCenter(float angle) const;
		void RotateCenter(float angle) const;

		// Box2D body related methods
		glm::vec2 GetLinearVelocity() const;
		void SetLinearVelocity(float x_mps, float y_mps) const;
		void SetLinearVelocityX(float mps) const;
		void SetLinearVelocityY(float mps) const;
		void ApplyLinearImpulse(const glm::vec2& impulse, const glm::vec2& point = {0.0f, 0.0f}) const;

		// Operator overloads
		operator uint32_t() const { return (uint32_t)m_Handle; }
		operator entt::entity() const { return m_Handle; }
		operator bool() const { return m_Handle != entt::null; }
		bool operator==(const Entity& other) const { return other.m_Handle == m_Handle; }
		bool operator!=(const Entity& other) const { return !(other == *this); }

	private:
		void DestroyAllScripts();

	private:
		entt::entity m_Handle = entt::null;
		Scene* m_Scene = nullptr;

		friend class Scene;
		friend class SceneSerializer;
		friend class EntityScript;
		friend class PhysicsWorld;
		friend class PhysicsContactListener;
		friend struct PhysicsContact;

		friend class InspectorPanel;
		friend class SceneViewportPanel;
	};

}
