// Header-only script example
#pragma once

class RotationScript : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(RotationScript)

	virtual void OnRegisterFields() override 
	{
		RegisterField(ScriptFieldType::Float, "RotationSpeed", &m_RotationSpeed);
	}

	virtual void OnUpdate(float ts) override 
	{
		RotateCenter(m_RotationSpeed * ts);
	}

private:
	float m_RotationSpeed = 1.0f;
};

