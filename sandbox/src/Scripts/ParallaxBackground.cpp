#include <Proton.h>
using namespace proton;

#include "ParallaxBackground.h"


void ParallaxBackground::OnRegisterFields()
{
	RegisterField(ScriptFieldType::Float, "ParallaxFactor", &m_ParallaxFactor);
	RegisterField(ScriptFieldType::Float, "PositionOffset", &m_PositionOffset);
}

bool ParallaxBackground::OnCreate()
{
	m_Scene = GetScene();
	auto& camera = m_Scene->GetPrimaryCamera();
	auto& sprite = GetComponent<SpriteComponent>().Sprite;

	m_SpriteAspectRatio = sprite.GetAspectRatio();
	m_CopiesCount = 1 + (uint32_t)ceil(camera.GetAspectRatio() / m_SpriteAspectRatio);
	DestroyChildEntities();

	for (uint32_t i = 0; i < m_CopiesCount; i++)
	{
		Entity copy = m_Scene->CreateEntity(GetTag() + "-" + std::to_string(i));
		copy.AddComponent<SpriteComponent>().Sprite.SetTexture(sprite.GetTexture());
		copy.GetTransform().WorldPosition.z = GetTransform().WorldPosition.z;
		AddChildEntity(copy);
		m_Copies.push_back(copy);
	}
	return true;
}

void ParallaxBackground::OnUpdate(float ts)
{
	const auto& camera = m_Scene->GetPrimaryCamera();
	float zoomLevel = camera.GetZoomLevel();
	float viewSize = camera.GetOrthographicSize() * zoomLevel;
	const glm::vec3& cameraPos = m_Scene->GetPrimaryCameraPosition();
	glm::vec2 scale = { viewSize * m_SpriteAspectRatio, viewSize };

	float offset = fmod(cameraPos.x * zoomLevel * m_ParallaxFactor, scale.x);
	float position = cameraPos.x - scale.x * m_CopiesCount / 2.0f + m_PositionOffset * zoomLevel;
	
	auto& transform = GetTransform();
	transform.Scale = scale;
	SetWorldPosition({ position - offset, cameraPos.y, transform.WorldPosition.z });

	for (Entity copy : m_Copies)
	{
		auto& transform = copy.GetTransform();
		position += scale.x;
		transform.Scale = scale;
		copy.SetWorldPosition({ position - offset, cameraPos.y, transform.WorldPosition.z });
	}
}
