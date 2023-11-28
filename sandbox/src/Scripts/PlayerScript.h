#pragma once

enum PlayerDirection : bool
{
	Right, Left
};

enum PlayerAnimation : uint32_t
{
	Idle, Run, Attack, Jump
};

// TODO: Remove this and add Character, CharacterController builtin scripts in the engine
class PlayerScript: public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(PlayerScript)

	virtual void OnRegisterFields() override;
	virtual void OnCreate() override;
	virtual void OnUpdate(float ts) override;

private:
	float m_PlayerSpeed = 5.0f;
	float m_JumpForce = 5.0f;

	Shared<SpriteAnimation> m_Animation;
	PlayerDirection m_Direction = Right;

	bool m_IsAttacking = false;
	bool m_IsJumping = true;
	float m_JumpDelay = 0.0f;

	b2Body* m_Body = nullptr;
	Entity m_FootSensor;
	uint32_t m_ContactCount = 0;
};
