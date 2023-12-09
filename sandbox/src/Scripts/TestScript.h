#pragma once
// Header-only script example

// Script for ScriptsAndPhysics scene
class TestScript : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(TestScript)

	virtual bool OnCreate() override
	{
		// Set collision callback function (lambda)
		auto& bc = GetComponent<BoxColliderComponent>();
		bc.ContactCallback.OnBegin = [&](PhysicsContact contact)
		{
			// Change color to the color of entity that collided with
			auto& sprite = contact.Other->GetComponent<SpriteComponent>();
			if (sprite.Color != glm::vec4{ 1.0f })
				GetComponent<SpriteComponent>().Color = sprite.Color;
		};
		return true;
	}

	virtual void OnUpdate(float ts) override
	{
		// Apply random impules in random intervals (1-3s)
		if (m_Timer == 0.0f)
		{
			m_Timer = Random::Float(1.0f, 3.0f);
			float ix = Random::Float(-200.0f, 200.0f);
			float iy = Random::Float(-200.0f, 200.0f);
			ApplyLinearImpulse({ ix, iy });
		}

		m_Timer = glm::max(m_Timer - ts, 0.0f);
	}

private:
	float m_Timer = 0.0f;
};
