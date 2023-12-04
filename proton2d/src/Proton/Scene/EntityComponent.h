#pragma once
#include "Proton/Scene/Entity.h"
#include "Proton/Physics/PhysicsWorld.h"

namespace proton {

	// AddComponent<RigidbodyComponent>
	template<>
	RigidbodyComponent& Entity::AddComponent() const
	{
		PT_CORE_ASSERT(!HasComponent<RigidbodyComponent>(), "Entity already has component!");
		auto& rb = m_Scene->m_Registry.emplace<RigidbodyComponent>(m_Handle);
		if (m_Scene->m_SceneState == SceneState::Play && m_Scene->m_PhysicsWorld->IsInitialized())
			m_Scene->m_PhysicsWorld->m_EntitiesToInitialize.push_back(*this);
		return rb;
	}

	// AddComponent<BoxColliderComponent>
	template<>
	BoxColliderComponent& Entity::AddComponent() const
	{
		PT_CORE_ASSERT(!HasComponent<BoxColliderComponent>(), "Entity already has component!");
		auto& bc = m_Scene->m_Registry.emplace<BoxColliderComponent>(m_Handle);
		if (m_Scene->m_SceneState == SceneState::Play && m_Scene->m_PhysicsWorld->IsInitialized())
			m_Scene->m_PhysicsWorld->m_EntitiesToInitialize.push_back(*this);
		return bc;
	}

	// AddComponent<ResizableSpriteComponent>
	template<>
	ResizableSpriteComponent& Entity::AddComponent() const
	{
		PT_CORE_ASSERT(!HasComponent<ResizableSpriteComponent>(), "Entity already has component!");
		PT_CORE_ASSERT(HasComponent<TransformComponent>(), "Entity does not have TransformComponent!");
		auto& sprite = m_Scene->m_Registry.emplace<ResizableSpriteComponent>(m_Handle);
		sprite.ResizableSprite.m_Transform = &GetComponent<TransformComponent>();
		return sprite;
	}

	// AddComponent<SpriteAnimationComponent>
	template<>
	SpriteAnimationComponent& Entity::AddComponent() const
	{
		PT_CORE_ASSERT(!HasComponent<SpriteAnimationComponent>(), "Entity already has component!");
		PT_CORE_ASSERT(HasComponent<SpriteComponent>(), "Entity must have SpriteComponent!");
		auto& sprite = GetComponent<SpriteComponent>().Sprite;
		PT_CORE_ASSERT(sprite.m_Spritesheet, "Entity must have Spritesheet Texture!");
		auto& component = m_Scene->m_Registry.emplace<SpriteAnimationComponent>(m_Handle, &sprite);
		return component;
	}

}
