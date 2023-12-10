#pragma once

enum PlayerState : uint16_t
{
	Idle, Run, Jump, Land
};

class Player : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(Player)

	virtual void OnRegisterFields() override;
	virtual bool OnCreate() override;
	virtual void OnUpdate(float ts) override;

private:
	bool IsTouchingGround() const { return *m_FootSensorContactCount > 0; }

private:
	float m_PlayerMaxSpeed = 6.0f;
	float m_JumpForce = 20.0f;
	float m_PlayerAcceleration = 40.0f;
	float m_GravityModifier = -10.0f;

	PlayerState m_State = Idle;
	SpriteAnimation* m_Animation;
	float m_Direction = 1.0f;
	float m_JumpTimer = 0.0f;

	uint32_t* m_FootSensorContactCount;
};
