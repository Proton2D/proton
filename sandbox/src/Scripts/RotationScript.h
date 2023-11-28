#pragma once
// Header-only script example

class RotationScript : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(RotationScript)

	virtual void OnRegisterFields() override
	{
		RegisterField(ScriptFieldType::Float, "RotationSpeed", &m_RotationSpeed);
	}

	virtual void OnCreate() override
	{
		m_Body = m_Entity.GetRuntimeBody();
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Body) 
		{
			m_Body->SetTransform(m_Body->GetPosition(),
				m_Body->GetAngle() + m_RotationSpeed * b2_pi * ts);
		}
		else
			m_Entity.GetTransform().Rotation += m_RotationSpeed;
	}

private:
	b2Body* m_Body = nullptr;
	float m_RotationSpeed = 0.5f;
};
