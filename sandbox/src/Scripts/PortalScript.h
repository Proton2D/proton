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
		AddComponent<SpriteAnimationComponent>();
		SpriteAnimation& animation = GetSpriteAnimation();

		animation.AddAnimation(0, 8);
		animation.PlayAnimation(0);
		animation.SetFPS(10);

		auto& bc = GetComponent<BoxColliderComponent>();
		bc.ContactCallback.OnBegin = [&](PhysicsContact contact) 
		{
			if (contact.Other->GetTag() == "Player")
			{
				std::string levelName = "level_" + std::to_string(m_TargetLevel);
				SceneManager::SetActiveScene(levelName)->BeginPlay();
			}
		};
		return true;
	}

private:
	int m_TargetLevel = 1;
};
