#pragma once

#include "Proton/Core/UUID.h"
#include "Proton/Graphics/Sprite.h"
#include "Proton/Graphics/ResizableSprite.h"
#include "Proton/Graphics/Camera.h"
#include "Proton/Graphics/SpriteAnimation.h"
#include "Proton/Physics/PhysicsCommon.h"

#include <entt/entity/entity.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <box2d/b2_body.h>

namespace proton {

	struct IDComponent
	{
		UUID ID;
	};

	struct TagComponent
	{
		std::string Tag;
	};

	// Use Entity::SetWorldPosition to modify world position manually
	struct TransformComponent
	{
		glm::vec3 WorldPosition { 0.0f, 0.0f, 0.0f };
		glm::vec3 LocalPosition { 0.0f, 0.0f, 0.0f };
		float Rotation { 0.0f };
		glm::vec2 Scale { 1.0f, 1.0f };
	};

	struct RelationshipComponent
	{
		uint32_t ChildrenCount = 0;
		entt::entity First  { entt::null };
		entt::entity Prev   { entt::null };
		entt::entity Next   { entt::null };
		entt::entity Parent { entt::null };
	};

	struct SpriteComponent
	{
		SpriteComponent(const std::string& filepath = std::string())
		{
			if (filepath.size())
				Sprite.SetTextureFromPath(filepath);
		}

		Sprite Sprite;
		// RGBA, range: 0.0f - 1.0f
		glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };
		float TilingFactor = 1.0f;
	};

	struct ResizableSpriteComponent
	{
		ResizableSprite ResizableSprite;
		// RGBA, range: 0.0f - 1.0f
		glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };
	};

	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;
	};

	class EntityScript; // forward declaration

	struct ScriptComponent
	{
		std::unordered_map<std::string, EntityScript*> Scripts;
	};

	struct CameraComponent
	{
		Camera Camera;
		glm::vec2 PositionOffset { 0.0f, 0.0f };
	};

	struct RigidbodyComponent
	{
		b2BodyType Type = b2_staticBody;
		bool FixedRotation = false;
	};

	struct BoxColliderComponent
	{
		glm::vec2 Size { 1.0f, 1.0f };
		glm::vec2 Offset { 0.0f, 0.0f };
		PhysicsMaterial Material;
		PhysicsContactCallback ContactCallback;
		b2Filter Filter;
		bool IsSensor = false;
	};

	struct CircleColliderComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 1.0f;
		PhysicsMaterial Material;
		PhysicsContactCallback ContactCallback;
		bool IsSensor = false;
	};

	struct SpriteAnimationComponent
	{
		SpriteAnimation SpriteAnimation;
	};
}
