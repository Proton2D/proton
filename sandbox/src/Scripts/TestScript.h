#pragma once
// Header-only script example

// Script for ScriptsAndPhysics scene
class TestScript : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(TestScript)

	virtual bool OnCreate() override
	{
		m_Body = m_Entity.GetRuntimeBody();
		auto& bc = m_Entity.GetComponent<BoxColliderComponent>();

		bc.ContactCallback.OnBeginContactFunction = [&](PhysicsContact contact)
		{
			auto& sprite = contact.Other->GetComponent<SpriteComponent>();
			if (sprite.Color != glm::vec4{ 1.0f })
				m_Entity.GetComponent<SpriteComponent>().Color = sprite.Color;
		};
		return true;
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Timer == 0.0f)
		{
			m_Timer = Random::Float(1.0f, 3.0f);
			float ix = Random::Float(-200.0f, 200.0f);
			float iy = Random::Float(-200.0f, 200.0f);
			m_Body->ApplyLinearImpulse({ ix, iy }, m_Body->GetWorldCenter(), true);
		}

		m_Timer = glm::max(m_Timer - ts, 0.0f);
	}

private:
	b2Body* m_Body;
	float m_Timer = 0.0f;
};
