#pragma once

class PortalScript : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(PortalScript)

	virtual void OnRegisterFields() override
	{
		RegisterField(ScriptFieldType::Int, "Target Level", &m_TargetLevel);
	}

	virtual bool OnCreate() override
	{
		m_SpriteAnimation = CreateSpriteAnimation();
		m_SpriteAnimation->AddAnimation(0, 8);
		m_SpriteAnimation->SetAnimation(0);
		m_SpriteAnimation->SetFPS(10);

		auto& bc = GetComponent<BoxColliderComponent>();
		bc.ContactCallback.OnBegin = [&](PhysicsContact contact) 
		{
			if (contact.Other->GetTag() == "Player")
			{
				std::string level = "level_" + std::to_string(m_TargetLevel);
				SceneManager::SetActiveScene(level, true);
			}
		};
		return true;
	}

private:
	SpriteAnimation* m_SpriteAnimation;
	int m_TargetLevel = 1;
};
