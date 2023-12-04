#include <Proton.h>
using namespace proton;

#include "PortalScript.h"


void PortalScript::OnRegisterFields()
{
	RegisterField(ScriptFieldType::Int, "Target Level", &m_TargetLevel);
}

bool PortalScript::OnCreate()
{
	m_SpriteAnimation = &m_Entity.AddComponent<SpriteAnimationComponent>().SpriteAnimation;
	m_SpriteAnimation->AddAnimation(0, 8);
	m_SpriteAnimation->SetAnimation(0);
	m_SpriteAnimation->SetFPS(10);

	auto& bc = m_Entity.GetComponent<BoxColliderComponent>();
	bc.ContactCallback.OnBeginContactFunction = [&](PhysicsContact contact) {
		if (contact.Other->GetTag() == "Player")
		{
			std::string level = "level_" + std::to_string(m_TargetLevel);
			SceneManager::Load(level, true);
			SceneManager::SetActiveScene(level);
		}
	};
	return true;
}
