#include <Proton.h>
using namespace proton;

#include "ParallaxBackground.h"


void ParallaxBackground::OnRegisterFields()
{
	RegisterField(ScriptFieldType::Float, "ParallaxFactor", &m_ParallaxFactor);
	RegisterField(ScriptFieldType::Float, "PositionOffset", &m_PositionOffset);
}

void ParallaxBackground::OnCreate()
{
	m_Scene = m_Entity.GetScene();
	auto& camera = m_Scene->GetPrimaryCamera();
	auto& sprite = m_Entity.GetComponent<SpriteComponent>().Sprite;

	m_SpriteAspectRatio = sprite.GetAspectRatio();
	m_CopiesCount = 1 + (uint32_t)ceil(camera->GetAspectRatio() / m_SpriteAspectRatio);
	m_Entity.DestroyChildEntities();

	for (uint32_t i = 0; i < m_CopiesCount; i++)
	{
		Entity e = m_Scene->CreateEntity(m_Entity.GetTag() + "-" + std::to_string(i));
		e.AddComponent<SpriteComponent>().Sprite.SetTexture(sprite.GetTexture());
		e.GetTransform().Position.z = m_Entity.GetTransform().Position.z;
		m_Entity.AddChildEntity(e);
		m_Copies.push_back(e); 
	}
}

void ParallaxBackground::OnUpdate(float ts)
{
	const auto& camera = m_Scene->GetPrimaryCamera();
	float zoomLevel = camera->GetZoomLevel();
	float viewSize = camera->GetOrthographicSize() * zoomLevel;
	const glm::vec3& cameraPos = m_Scene->GetPrimaryCameraPosition();
	glm::vec2 scale = { viewSize * m_SpriteAspectRatio, viewSize };

	float offset = fmod(cameraPos.x * zoomLevel * m_ParallaxFactor, scale.x);
	float position = cameraPos.x - scale.x * m_CopiesCount / 2.0f + m_PositionOffset * zoomLevel;
	
	auto& transform = m_Entity.GetTransform();
	transform.Scale = scale;
	transform.Position.x = position - offset;
	transform.Position.y = cameraPos.y;

	for (Entity copy : m_Copies)
	{
		auto& transform = copy.GetTransform();
		position += scale.x;
		transform.Scale = scale;
		transform.Position.x = position - offset;
		transform.Position.y = cameraPos.y;
	}
}
