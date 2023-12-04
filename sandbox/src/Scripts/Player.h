#pragma once

enum PlayerDirection : bool
{
	Right, Left
};

enum PlayerAnimation : uint32_t
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
	float m_PlayerSpeed = 6.0f;
	float m_JumpForce = 45.0f;

	SpriteAnimation* m_Animation;
	PlayerDirection m_Direction = Right;
	b2Body* m_Body = nullptr;

	bool m_IsJumping = false;
	bool m_IsLanding = false;
	float m_JumpDelay = 0.0f;

	Entity m_FootSensor;
	uint32_t m_ContactCount = 0;
};
