#include <Proton.h>
using namespace proton;

#include "PlayerScript.h"


void PlayerScript::OnRegisterFields()
{
	RegisterField(ScriptFieldType::Float, "PlayerSpeed", &m_PlayerSpeed);
	RegisterField(ScriptFieldType::Float, "JumpForce", &m_JumpForce);
}

void PlayerScript::OnCreate()
{
	m_Body = m_Entity.GetRuntimeBody();
	// Create animations
	m_Animation = m_Entity.AddComponent<SpriteAnimationComponent>().SpriteAnimation;
	m_Animation->SetFPS(10);
	m_Animation->AddAnimation(Idle, 10);
	m_Animation->AddAnimation(Run, 10);
	m_Animation->AddAnimation(Attack, 10);
	m_Animation->AddAnimation(Jump, 3);
	m_Animation->SetAnimation(Idle, Right);

	// Foot sensor detects if player is on the ground
	proton::UUID playerUUID = m_Entity.GetUUID();
	m_FootSensor = m_Entity.GetScene()->FindByTag("FootSensor");
	if (m_FootSensor)
	{
		auto& bc = m_FootSensor.GetComponent<BoxColliderComponent>();

		bc.ContactCallback.OnBeginContactFunction = [&, playerUUID](PhysicsContactInfo info)
		{
			if (info.OtherUUID != playerUUID)
				m_ContactCount++;
		};

		bc.ContactCallback.OnEndContactFunction = [&, playerUUID](PhysicsContactInfo info)
		{
			if (info.OtherUUID != playerUUID)
				m_ContactCount--;
		};
	}
	else {
		// TODO: Create in OnCreate, remove from the Player prefab
		PT_ERROR("Player has no FootSensor");
	}
}

void PlayerScript::OnUpdate(float ts)
{
	// TODO: Add world/local position feature for child entities in engine and remove this
	const glm::vec3 playerPos = m_Entity.GetTransform().Position;
	m_FootSensor.GetRuntimeBody()->SetTransform({ playerPos.x, playerPos.y }, 0.0f);

	// === Attack (Space) ===
	bool space = Input::IsKeyPressed(Key::Space);
	if (m_IsAttacking)
	{
		if (!space && m_Animation->GetProgress() >= 0.5f)
		{
			// Stop attacking
			m_Animation->StartAnimation(Idle, m_Direction);
			m_Animation->SetPlayMode(AnimationPlayMode::REPEAT);
			m_IsAttacking = false;
		}
		else if (space && m_Animation->FinishedPlaying())
			m_Animation->Replay();
	}
	if (!m_IsAttacking)
	{
		if (space && !m_IsJumping)
		{
			// Begin attacking
			m_Animation->StartAnimation(Attack, m_Direction);
			m_Animation->SetPlayMode(AnimationPlayMode::PLAY_ONCE);
			m_Animation->Replay();
			m_IsAttacking = true;
		}
	} 

	// === Movement (A, D) ===
	if (Input::IsKeyPressed(Key::D))
	{
		m_Direction = Right;
		if (m_IsAttacking) // Walking while attacking
		{
			m_Entity.SetVelocityX(m_PlayerSpeed / 2);
			m_Animation->SetMirrorFlip(false);
		}
		else // Running
		{
			m_Entity.SetVelocityX(m_PlayerSpeed);
			if (!m_IsJumping)
				m_Animation->StartAnimation(Run);
			m_Animation->SetMirrorFlip(false);
		}
	}
	else if (Input::IsKeyPressed(Key::A))
	{
		m_Direction = Left;
		if (m_IsAttacking) // Walking while attacking
		{
			m_Entity.SetVelocityX(-m_PlayerSpeed / 2);
			m_Animation->SetMirrorFlip(true);
		}
		else // Running
		{
			m_Entity.SetVelocityX(-m_PlayerSpeed);
			if (!m_IsJumping)
				m_Animation->StartAnimation(Run);
			m_Animation->SetMirrorFlip(true);
		}
	}
	else  
	{
		m_Entity.SetVelocityX(0.0f);
		if (!m_IsAttacking && !m_IsJumping) // Idle
			m_Animation->SetAnimation(Idle, m_Direction);
	}

	// === Jumping (W) ===

	// Check if jumping / falling ended
	if (m_IsJumping && m_JumpDelay == 0.0f && m_ContactCount > 0)
	{
		m_IsJumping = false;
		m_JumpDelay = 0.1f;
		m_Animation->SetAnimation(Idle, m_Direction);
	}
	// Try jump
	if (!m_IsAttacking && Input::IsKeyPressed(Key::W))
	{
		if (m_ContactCount > 0 && m_JumpDelay == 0.0f)
		{
			m_IsJumping = true;
			m_JumpDelay = 0.1f;
			m_Entity.ApplyImpulse({ 0.0f,  m_JumpForce });
		}
	}
	// Set jumping / falling animation
	if (m_IsJumping || m_ContactCount == 0)
	{
		m_Animation->SetAnimation(Jump, m_Direction);
		m_IsJumping = true;
	}

	m_JumpDelay = glm::max(m_JumpDelay - ts, 0.0f);
}
