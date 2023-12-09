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

	virtual void OnImGuiRender() override;

private:
	float m_PlayerMaxSpeed = 6.0f;
	float m_JumpForce = 20.0f;
	float m_PlayerAcceleration = 40.0f;
	float m_GravityModifier = -10.0f;

	SpriteAnimation* m_Animation;
	PlayerDirection m_Direction = Right;

	float m_PlayerHorizontalVelocity = 0.0f;
	bool m_IsJumping = false;
	bool m_IsLanding = false;
	float m_JumpDelay = 0.0f;
	float m_LandDelay = 0.0f;

	Entity m_FootSensor;
	uint32_t m_ContactCount = 0;
};
