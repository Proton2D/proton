#include <Proton.h>
using namespace proton;

#include "MainLayer.h"

// Header-only scripts must be compiled somewhere
#include "Scripts/RotationScript.h" 
#include "Scripts/TestScript.h"


void MainLayer::OnCreate()
{
	SceneManager::Load("level_1");
	SceneManager::Load("level_2");
	SceneManager::SetActiveScene("level_1");
}

void MainLayer::OnUpdate(float ts)
{
}

void MainLayer::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	Scene* scene = SceneManager::GetActiveScene();

	dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == Key::R)
		{
			SpawnRandomBox(scene->GetCursorWorldPosition());
		}
		return false;
	});
}

Entity MainLayer::SpawnRandomBox(const glm::vec2& position)
{
	Scene* scene = SceneManager::GetActiveScene();
	Entity entity = scene->CreateEntity("Random Box");

	entity.AddComponent<RigidbodyComponent>().Type = b2_dynamicBody;
	entity.AddComponent<BoxColliderComponent>();

	auto& transform = entity.GetComponent<TransformComponent>();
	transform.WorldPosition = { position.x, position.y, 0 };
	transform.Rotation = Random::Float(0.0f, 80.0f);
	float scale = Random::Float(1.0f, 1.5f);
	transform.Scale = { scale, scale };

	auto& sprite = entity.AddComponent<SpriteComponent>("box.png");
	sprite.Color.r = Random::Float(0.0f, 1.0f);
	sprite.Color.g = Random::Float(0.0f, 1.0f);
	sprite.Color.b = Random::Float(0.0f, 1.0f);

	// TODO: Add an entity queue and remove this
	scene->CreateRuntimeBody(entity);
	
	return entity;
}
