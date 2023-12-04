#include <Proton.h>
using namespace proton;

#include "Player.h"

void Player::OnRegisterFields()
{
	RegisterField(ScriptFieldType::Float, "PlayerSpeed", &m_PlayerSpeed);
	RegisterField(ScriptFieldType::Float, "JumpForce", &m_JumpForce);
}

bool Player::OnCreate()
{
	if (!GetScene()->IsPhysicsEnabled())
	{
		PT_ERROR("[PlayerScript]: Physics is not enabled!");
		return false;
	}

	m_Body = m_Entity.GetRuntimeBody();

	// Create animations
	m_Animation = &m_Entity.AddComponent<SpriteAnimationComponent>().SpriteAnimation;
	m_Animation->SetFPS(12);
	m_Animation->AddAnimation(Idle, 10);
	m_Animation->AddAnimation(Run, 8);
	m_Animation->AddAnimation(Jump, 3);
	m_Animation->AddAnimation(Land, 9);
	m_Animation->SetAnimation(Idle, Right);

	// Foot sensor detects if player is the air
	m_FootSensor = GetScene()->CreateEntity("FootSensor");
	m_Entity.AddChildEntity(m_FootSensor, false);
	auto& bc = m_FootSensor.AddComponent<BoxColliderComponent>();
	bc.Size = { 0.32f, 0.15f };
	bc.Offset = { 0.0f, -0.8f };
	bc.IsSensor = true;

	// Set On Begin Contact callback function
	bc.ContactCallback.OnBeginContactFunction = [&](PhysicsContact contact)
	{
		if (*contact.Other != m_Entity)
			m_ContactCount++;
	};
	// Set On End Contact callback function
	bc.ContactCallback.OnEndContactFunction = [&](PhysicsContact contact)
	{
		if (*contact.Other != m_Entity)
			m_ContactCount--;
	};

	return true;
}

void Player::OnUpdate(float ts)
{
	// ======================== Movement (A, D) ========================

	if (Input::IsKeyPressed(Key::D))
	{
		if (m_IsLanding)
		{
			// Cancel landing animation
			m_Animation->SetPlayMode(AnimationPlayMode::REPEAT);
			m_IsLanding = false;
		}
		// Move right
		m_Direction = Right;
		m_Entity.SetVelocityX(m_PlayerSpeed);
		if (!m_IsJumping)
			m_Animation->StartAnimation(Run);
	}
	else if (Input::IsKeyPressed(Key::A))
	{
		if (m_IsLanding)
		{
			// Cancel landing animation
			m_Animation->SetPlayMode(AnimationPlayMode::REPEAT);
			m_IsLanding = false;
		}
		// Move left
		m_Direction = Left;
		m_Entity.SetVelocityX(-m_PlayerSpeed);
		if (!m_IsJumping)
			m_Animation->StartAnimation(Run);
	}
	else
	{
		// Idle
		m_Entity.SetVelocityX(0.0f);
		if (!m_IsJumping && !m_IsLanding)
			m_Animation->SetAnimation(Idle);
	}

	// ======================== Jumping (W) ========================

	// Check if jumping / falling ended
	if (m_IsJumping && m_JumpDelay == 0.0f && m_ContactCount > 0)
	{
		m_IsLanding = true;
		m_IsJumping = false;
		m_JumpDelay = 0.2f;
		m_Animation->SetAnimation(Land);
		m_Animation->SetPlayMode(AnimationPlayMode::PLAY_ONCE);
	}

	// Landing animation
	if (m_IsLanding && m_Animation->FinishedPlaying())
	{
		m_IsLanding = false;
		m_Animation->SetAnimation(Idle);
		m_Animation->SetPlayMode(AnimationPlayMode::REPEAT);
	}

	// Try jump
	if (Input::IsKeyPressed(Key::W) && m_ContactCount > 0 && m_JumpDelay == 0.0f)
	{
		m_IsJumping = true;
		m_JumpDelay = 0.2f;
		m_Entity.ApplyImpulse({ 0.0f,  m_JumpForce });
		m_Animation->SetAnimation(Jump);
		m_Animation->SetPlayMode(AnimationPlayMode::PAUSED, true, 0);
	}

	// Set jumping / falling animation
	if (m_IsJumping || m_ContactCount == 0)
	{
		float velocity = m_Body->GetLinearVelocity().y;
		
		if (!m_IsJumping) 
		{
			m_Animation->SetAnimation(Jump);
			m_Animation->SetPlayMode(AnimationPlayMode::PAUSED, true, 1);
		}
		
		if (!m_JumpDelay)
		{
			if (velocity > 0.0f)
				m_Animation->SetPlayMode(AnimationPlayMode::PAUSED, true, 1);
			else
				m_Animation->SetPlayMode(AnimationPlayMode::PAUSED, true, 2);
		}

		m_IsJumping = true;

		// Modify jump velocity
		if (velocity < 0.05f && velocity > 0.0f)
			m_Body->ApplyLinearImpulseToCenter({ 0.0f,  -m_JumpForce / 7.0f }, true);
		else if (velocity > m_JumpForce / 5.0f)
			m_Body->ApplyLinearImpulseToCenter({ 0.0f,  -m_JumpForce / 6.0f }, true);
	}

	m_JumpDelay = glm::max(m_JumpDelay - ts, 0.0f);
	m_Animation->SetMirrorFlip(m_Direction);
}
