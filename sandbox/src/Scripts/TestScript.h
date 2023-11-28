#pragma once
// Header-only script example

// Script for ScriptsAndPhysics scene
class TestScript : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(TestScript)

	virtual void OnCreate() override
	{
		m_Body = m_Entity.GetRuntimeBody();
		auto& bc = m_Entity.GetComponent<BoxColliderComponent>();

		bc.ContactCallback.OnBeginContactFunction = [&](PhysicsContactInfo info)
		{
			Entity e = m_Entity.GetScene()->FindByID(info.OtherUUID);
			auto& sprite = e.GetComponent<SpriteComponent>();
			if (sprite.Color != glm::vec4{1.0f})
				m_Entity.GetComponent<SpriteComponent>().Color = sprite.Color;
		};
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Timer == 0.0f)
		{
			float ix = Random::Float(-200.0f, 200.0f);
			float iy = Random::Float(-200.0f, 200.0f);
			m_Body->ApplyLinearImpulse({ ix, iy }, m_Body->GetWorldCenter(), true);
			m_Timer = Random::Float(1.0f, 3.0f);
		}

		m_Timer = glm::max(m_Timer - ts, 0.0f);
	}

private:
	b2Body* m_Body;
	float m_Timer = 0.0f;
};
