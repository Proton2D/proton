#include <Proton.h>
using namespace proton;

#include "Player.h"

static constexpr float s_DefaultJumpDelay = 0.2f;

void Player::OnRegisterFields()
{
	RegisterField(ScriptFieldType::Float, "PlayerSpeed", &m_PlayerHorizontalVelocity);
	RegisterField(ScriptFieldType::Float, "PlayerAcceleration", &m_PlayerAcceleration);
	RegisterField(ScriptFieldType::Float, "JumpForce", &m_JumpForce);
	RegisterField(ScriptFieldType::Float, "GravityModifier", &m_GravityModifier);
}

bool Player::OnCreate()
{

	if (!GetScene()->IsPhysicsEnabled())
	{
		PT_ERROR_FUNCSIG("Physics is not enabled!");
		return false;
	}

	// Set up animations
	m_Animation = CreateSpriteAnimation();
	m_Animation->SetFPS(8);
	m_Animation->AddAnimation(Idle, 10);
	m_Animation->AddAnimation(Run, 8);
	m_Animation->AddAnimation(Jump, 3);
	m_Animation->AddAnimation(Land, 9);
	m_Animation->SetAnimation(Idle, Right);

	// Create foot sensor
	// Foot sensor detects if player is touching the ground
	m_FootSensor = GetScene()->CreateEntity("FootSensor");
	AddChildEntity(m_FootSensor, false);
	auto& footSensor = m_FootSensor.AddComponent<BoxColliderComponent>();
	footSensor.Size = { 0.38f, 0.38f };
	footSensor.Offset = { 0.0f, -0.8f };
	footSensor.IsSensor = true;

	// Set foot sensor contact callback functions (box collider collisions)...
	// On Begin Contact callback
	footSensor.ContactCallback.OnBegin = [&](PhysicsContact contact)
	{
		if (!contact.OtherCompare(this)) // ignore player body collider
			m_ContactCount++;
	};
	// On End Contact callback
	footSensor.ContactCallback.OnEnd = [&](PhysicsContact contact)
	{
		if (!contact.OtherCompare(this)) // ignore player body collider
			m_ContactCount--;
	};

	return true;
}

void Player::OnUpdate(float ts)
{
	// ------------- Sideways Movement -------------
	bool moveRight = Input::IsKeyPressed(Key::D);
	bool moveLeft = Input::IsKeyPressed(Key::A);

	// Run animation
	if ((moveRight || moveLeft) && m_JumpDelay < 0.1f)
	{
		if (m_IsLanding)
		{
			// Cancel landing animation
			m_IsLanding = false;
			m_Animation->SetPlayMode(AnimationPlayMode::REPEAT);
		}
		if (!m_IsJumping)
			m_Animation->StartAnimation(Run);
	}

	// Set horizontal velocity
	float velocityDirectionModifier = 0.0f;
	if (moveRight)
	{
		m_Direction = Right;
		velocityDirectionModifier = 1.0f;

	}
	else if (moveLeft)
	{
		m_Direction = Left;
		velocityDirectionModifier = -1.0f;
	}
	else // Player idle
	{
		if (!m_IsJumping && !m_IsLanding)
			m_Animation->SetAnimation(Idle);
	}

	// Smooth acceleration
	float horizontalVelocity = 0.0f;
	if (velocityDirectionModifier != 0.0f)
		horizontalVelocity = m_PlayerHorizontalVelocity + m_PlayerAcceleration * velocityDirectionModifier * ts;
	m_PlayerHorizontalVelocity = glm::min(glm::max(horizontalVelocity,-m_PlayerMaxSpeed), m_PlayerMaxSpeed);
	SetLinearVelocityX(m_PlayerHorizontalVelocity);

	// ------------- Jumping / Landing -------------
	// Start landing animation
	if (m_IsJumping && m_JumpDelay == 0.0f && m_ContactCount > 0)
	{
		if (!m_LandDelay)
		{
			m_Animation->SetAnimation(Land);
			m_Animation->SetPlayMode(AnimationPlayMode::PLAY_ONCE);
		}
		m_LandDelay = 1.0f;
		m_JumpDelay = s_DefaultJumpDelay;
		m_IsLanding = true;
		m_IsJumping = false;
	}

	// Stop landing animation
	if (m_IsLanding && m_Animation->FinishedPlaying())
	{
		m_IsLanding = false;
		m_Animation->SetAnimation(Idle);
		m_Animation->SetPlayMode(AnimationPlayMode::REPEAT);
	}

	// Player Jump
	if (Input::IsKeyPressed(Key::W) && m_ContactCount > 0 && m_JumpDelay == 0.0f)
	{
		m_JumpDelay = s_DefaultJumpDelay;
		m_IsJumping = true;
		m_Animation->SetAnimation(Jump);
		m_Animation->SetPlayMode(AnimationPlayMode::PAUSED, 0);
		ApplyLinearImpulse({ 0.0f,  m_JumpForce });
	}

	// Player is in the air: set jumping or falling animation frame 
	float verticalVelocity = GetLinearVelocity().y;
	if (m_ContactCount == 0)
	{		
		if (!m_IsJumping) 
			m_Animation->SetAnimation(Jump);
		
		if (verticalVelocity > 0.0f)
			m_Animation->SetAnimationFrame(1);
		else
			m_Animation->SetAnimationFrame(2);

		// Modify the velocity to make jump less floaty
		if (verticalVelocity < 0.05f && verticalVelocity > 0.0f)
			ApplyLinearImpulse({ 0.0f,  m_GravityModifier });
		
		m_IsJumping = true;
	}

	m_JumpDelay = glm::max(m_JumpDelay - ts, 0.0f);
	m_LandDelay = glm::max(m_LandDelay - ts, 0.0f);
	m_Animation->SetMirrorFlip(m_Direction);
}

void Player::OnImGuiRender()
{
#ifdef PT_EDITOR
	if (GetScene()->GetSceneState() == SceneState::Play)
	{
		glm::vec2 velocity = GetLinearVelocity();
		ImGui::Text("Velocity (%.2f, %.2f)", velocity.x, velocity.y);
	}
#endif
}
