#include <Proton.h>
using namespace proton;

#include "MainLayer.h"

// Header-only scripts must be compiled somewhere
#include "Scripts/PortalScript.h"
#include "Scripts/ColorHueAnimation.h"
#include "Scripts/RotationScript.h" 
#include "Scripts/TestScript.h"


void MainLayer::OnCreate()
{
	SceneManager::Load("level_1");
	SceneManager::Load("level_2");
	SceneManager::Load("level_3");
	SceneManager::SetActiveScene("level_1");
}

void MainLayer::OnUpdate(float ts)
{
}

void MainLayer::OnEvent(Event& e)
{
	Scene* scene = SceneManager::GetActiveScene();
	EventDispatcher dispatcher(e);

	dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == Key::E)
			SpawnRandomBox(scene->GetCursorWorldPosition());

		return false;
	});
}

void MainLayer::SpawnRandomBox(const glm::vec2& position)
{
	Scene* scene = SceneManager::GetActiveScene();
	Entity entity = scene->CreateEntity("Random Box");

	auto& transform = entity.GetComponent<TransformComponent>();
	transform.WorldPosition = { position.x, position.y, 0 };
	transform.Rotation = Random::Float(0.0f, 80.0f);

	auto& sprite = entity.AddComponent<SpriteComponent>("box.png");
	sprite.Color.r = Random::Float(0.0f, 1.0f);
	sprite.Color.g = Random::Float(0.0f, 1.0f);
	sprite.Color.b = Random::Float(0.0f, 1.0f);

	entity.AddComponent<RigidbodyComponent>().Type = b2_dynamicBody;
	entity.AddComponent<BoxColliderComponent>();
}
