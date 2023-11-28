#include <Proton.h>
using namespace proton;

#include "PortalScript.h"


void PortalScript::OnRegisterFields()
{
	RegisterField(ScriptFieldType::Int, "Target Level", &m_TargetLevel);
}

void PortalScript::OnCreate()
{
	m_SpriteAnimation = m_Entity.AddComponent<SpriteAnimationComponent>().SpriteAnimation;
	m_SpriteAnimation->AddAnimation(0, 8);
	m_SpriteAnimation->SetAnimation(0);
	m_SpriteAnimation->SetFPS(10);

	auto& bc = m_Entity.GetComponent<BoxColliderComponent>();
	bc.ContactCallback.OnBeginContactFunction = [&](PhysicsContactInfo contact) {
		if (m_Entity.GetScene()->FindByID(contact.OtherUUID).GetTag() == "Player")
		{
			std::string level = "level_" + std::to_string(m_TargetLevel);
			if (!SceneManager::IsLoaded(level))
				SceneManager::Load(level);
			SceneManager::SetActiveScene(level)->BeginPlay();
		}
	};
}
