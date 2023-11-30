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
			PT_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_Handle);
		}

		template <typename T, typename... Types>
		T& AddComponent(Types&& ...args) const
		{
			PT_ASSERT(!HasComponent<T>(), "Entity already have component!");
			return m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Types>(args)...);
		}

		// ResizableSpriteComponent override of AddComponent
		template<>
		ResizableSpriteComponent& AddComponent() const
		{
			PT_ASSERT(!HasComponent<ResizableSpriteComponent>(), "Entity already has component!");
			PT_ASSERT(HasComponent<TransformComponent>(), "Entity does not have TransformComponent!");
			auto& sprite = m_Scene->m_Registry.emplace<ResizableSpriteComponent>(m_Handle);
			sprite.ResizableSprite.m_Transform = &GetComponent<TransformComponent>();
			return sprite;
		}

		// SpriteAnimationComponent override of AddComponent
		template<>
		SpriteAnimationComponent& AddComponent() const
		{
			PT_ASSERT(!HasComponent<SpriteAnimationComponent>(), "Entity already has component!");
			PT_ASSERT(HasComponent<SpriteComponent>(), "Entity must have SpriteComponent!");
			auto& sprite = GetComponent<SpriteComponent>().Sprite;
			PT_ASSERT(sprite.m_Spritesheet, "Entity must have Spritesheet Texture!");

			auto& fb = m_Scene->m_Registry.emplace<SpriteAnimationComponent>(m_Handle);
			fb.SpriteAnimation = MakeShared<SpriteAnimation>(&sprite);
			return fb;
		}

		// Add script to Entity and return instance
		template <typename TScriptClass>
		EntityScript* AddScript() const
		{
			if (!HasComponent<ScriptComponent>())
				AddComponent<ScriptComponent>();

			auto& component = GetComponent<ScriptComponent>();
			std::string className{ TScriptClass::__ScriptClassName };
			PT_ASSERT(component.Scripts.find(className) == component.Scripts.end(), "The script is already attached to an Entity!");

			EntityScript*& scriptInstance = component.Scripts[className];
			scriptInstance = new TScriptClass();
			scriptInstance->OnRegisterFields();
			return scriptInstance;
		}

		// Remove script from Entity
		void RemoveScript(const std::string& scriptClassName);

		// Remove component from Entity
		template <typename T>
		void RemoveComponent()
		{
			PT_ASSERT(HasComponent<T>(), "Entity does not have a component!");
			
			if (std::is_base_of<ScriptComponent, T>::value)
				TerminateScripts();

			if (std::is_base_of<CameraComponent, T>::value
				&& m_Scene->m_PrimaryCameraEntity == *this)
			{
				PT_CORE_WARN("[Entity::RemoveComponent<CameraComponent>] Entity with Scene Primary Camera has been removed!");
				m_Scene->m_PrimaryCameraEntity = entt::null;
				m_Scene->m_PrimaryCamera = nullptr;
			}

			m_Scene->m_Registry.remove<T>(m_Handle);
		}

		// Check if Entity has given component
		template <typename T>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.any_of<T>(m_Handle);
		}

		// Check if Entity has given set of components
		template <typename... TComponents>
		bool HasComponents() const
		{
			return m_Scene->m_Registry.all_of<TComponents...>(m_Handle);
		}

		// Get TransformComponent
		TransformComponent& GetTransform();

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
		void AddChildEntity(Entity child);

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

		friend class InspectorPanel;
		friend class SceneViewportPanel;
	};

}
