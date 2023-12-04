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

		template <typename T>
		T& GetComponent() const
		{
			PT_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_Handle);
		}

		template <typename T, typename... Types>
		T& AddComponent(Types&& ...args) const
		{
			PT_CORE_ASSERT(!HasComponent<T>(), "Entity already have component!");
			return m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Types>(args)...);
		}

		template <typename T>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.any_of<T>(m_Handle);
		}

		template <typename... TComponents>
		bool HasComponents() const
		{
			return m_Scene->m_Registry.all_of<TComponents...>(m_Handle);
		}

		// Remove component from Entity
		template <typename T>
		void RemoveComponent()
		{
			PT_CORE_ASSERT(HasComponent<T>(), "Entity does not have a component!");

			if (std::is_base_of<ScriptComponent, T>::value)
				TerminateScripts();

			if (std::is_base_of<CameraComponent, T>::value
				&& m_Scene->m_PrimaryCameraEntity == *this)
			{
				PT_CORE_WARN("[Entity::RemoveComponent<CameraComponent>] Scene Primary Camera has been removed!");
				m_Scene->m_PrimaryCameraEntity = entt::null;
				m_Scene->m_PrimaryCamera = nullptr;
			}

			m_Scene->m_Registry.remove<T>(m_Handle);
		}

		// Add script to Entity and return instance
		template <typename TScriptClass>
		EntityScript* AddScript() const
		{
			if (!HasComponent<ScriptComponent>())
				AddComponent<ScriptComponent>();

			auto& component = GetComponent<ScriptComponent>();
			std::string className{ TScriptClass::__ScriptClassName };
			PT_CORE_ASSERT(component.Scripts.find(className) == component.Scripts.end(), "The script is already attached to an Entity!");

			EntityScript*& scriptInstance = component.Scripts[className];
			scriptInstance = new TScriptClass();
			scriptInstance->OnRegisterFields();
			return scriptInstance;
		}

		// Definitions in EntityComponent.h

		// AddComponent<ResizableSpriteComponent>
		template<> ResizableSpriteComponent& AddComponent() const;

		// AddComponent<RigidbodyComponent>
		template<> RigidbodyComponent& AddComponent() const;

		// AddComponent<BoxColliderComponent>
		template<> BoxColliderComponent& AddComponent() const;

		// AddComponent<SpriteAnimationComponent>
		template<> SpriteAnimationComponent& AddComponent() const;


		// Remove script from Entity
		void RemoveScript(const std::string& scriptClassName);

		// Get TransformComponent
		TransformComponent& GetTransform();

		// Set position relative to world center
		// Updates World and Local position
		void SetWorldPosition(const glm::vec3& position);
		
		// Set position relative to parent entity position
		// Updates World and Local position
		void SetLocalPosition(const glm::vec3& position);

		// Get pointer to scene
		Scene* GetScene() { return m_Scene; }

		// Get Entity unique identifier
		UUID GetUUID() const;

		// Get Entity tag stored in TagComponent
		const std::string& GetTag() const;

		// Check if Entity is valid
		bool IsValid();

		// Destroy Entity and it's child entities
		void Destroy();

		// Add child Entity given as parameter
		void AddChildEntity(Entity child, bool refreshChildWorldPosition = true);

		// Destroy all child entities
		void DestroyChildEntities();

		// Detach Entity from parent Entity and move to scene root
		void PopHierarchy();

		// Check if Entity is parent of a given Entity
		bool IsParentOf(Entity entity);

		// ============== Box2D Rigidbody related ==============

		// RigidbodyComponent required
		b2Body* GetRuntimeBody();

		// RigidbodyComponent required
		void SetVelocity(float x_mps, float y_mps);

		// RigidbodyComponent required
		void SetVelocityX(float mps);

		// RigidbodyComponent required
		void SetVelocityY(float mps);

		// RigidbodyComponent required
		glm::vec2 GetVelocity();

		// RigidbodyComponent required
		void ApplyImpulse(const glm::vec2& impulse);

		// ======================================================

		// Operator overloads
		operator uint32_t() const { return (uint32_t)m_Handle; }
		operator entt::entity() const { return m_Handle; }
		operator bool() const { return m_Handle != entt::null; }
		bool operator==(const Entity& other) const { return other.m_Handle == m_Handle; }
		bool operator!=(const Entity& other) const { return !(other == *this); }

	private:
		void TerminateScripts();

	private:
		Scene* m_Scene = nullptr;
		entt::entity m_Handle = entt::null;

		friend class Scene;
		friend class SceneSerializer;
		friend class EntityScript;
		friend class PhysicsWorld;
		friend class PhysicsContactListener;

		friend class InspectorPanel;
		friend class SceneViewportPanel;
	};

}
