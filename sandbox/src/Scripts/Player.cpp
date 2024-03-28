#include <Proton.h>
using namespace proton;

#include "Player.h"

// Internal script parameters
static constexpr float s_JumpDelay = 0.2f;
static constexpr float s_JumpFrameSwitchTime = 0.3f;
static constexpr float s_LandAnimationDelay = 0.5f;
static constexpr float s_LandAnimationCancelTime = 0.2f;

void Player::OnRegisterFields()
{
	RegisterField(ScriptFieldType::Float, "PlayerMaxSpeed", &m_PlayerMaxSpeed);
	RegisterField(ScriptFieldType::Float, "PlayerAcceleration", &m_PlayerAcceleration);
	RegisterField(ScriptFieldType::Float, "JumpForce", &m_JumpForce);
	RegisterField(ScriptFieldType::Float, "GravityModifier", &m_GravityModifier);
}

bool Player::OnCreate()
{
	// Set up animations
	AddComponent<SpriteAnimationComponent>();
	SpriteAnimation& animation = GetSpriteAnimation();

	animation.AddAnimation(Idle, 10, AnimationPlayMode::REPEAT);
	animation.AddAnimation(Run,   8, AnimationPlayMode::REPEAT);
	animation.AddAnimation(Jump,  3, AnimationPlayMode::PAUSED);
	animation.AddAnimation(Land,  9, AnimationPlayMode::PLAY_ONCE);
	animation.SetFPS(8);

	// Foot sensor is used to detect if player is touching the ground
	Entity footSensor = CreateChildEntity("FootSensor");
	auto& bc = footSensor.AddComponent<BoxColliderComponent>();
	bc.Size = { 0.38f, 0.38f };
	bc.Offset = { 0.0f, -0.8f };
	bc.IsSensor = true;
	m_FootSensorContactCount = &bc.ContactCallback.ContactCount;

	return true;
}

void Player::OnUpdate(float ts)
{
	// Get SpriteAnimation object reference
	SpriteAnimation& animation = GetSpriteAnimation();

	// Poll key states for player movement
	bool moveRight = Input::IsKeyPressed(Key::D);
	bool moveLeft = Input::IsKeyPressed(Key::A);
	bool jump = Input::IsKeyPressed(Key::W);
	bool move = moveRight || moveLeft;
	
	// Set player direction (right: 1.0, left: -1.0f)
	m_Direction = moveRight ? 1.0f : (moveLeft ? -1.0f : m_Direction);

	// Set horizontal velocity (acceleration)
	SetLinearVelocityX(!move ? 0.0f : glm::clamp(
		GetLinearVelocity().x + m_PlayerAcceleration * m_Direction * ts,
		-m_PlayerMaxSpeed, m_PlayerMaxSpeed));
	
	// Set player state to Run when key is pressed and player is not in the air
	if (move && m_State != Jump && m_JumpTimer >= s_LandAnimationCancelTime)
		m_State = Run;
	// Set player state to Idle when stopped running or landing animation finished playing
	else if (m_State == Run || (m_State == Land && animation.FinishedPlaying()))
		m_State = Idle;

	// Start landing animation
	if (m_State == Jump && IsTouchingGround())
	{
		m_State = m_JumpTimer >= s_LandAnimationDelay ? Land : Idle;
		m_JumpTimer = 0.0f;
	}

	// Player pressed a jump key
	if (jump && IsTouchingGround() && m_JumpTimer >= s_JumpDelay)
	{
		ApplyLinearImpulse({ 0.0f,  m_JumpForce });
		m_JumpTimer = 0.0f;
		m_State = Jump;
	}

	// Player is in the air: Set jump or fall animation frame 
	float velocity = GetLinearVelocity().y;
	if (!IsTouchingGround())
	{
		// Modify vertical velocity to make jump feel less floaty
		if (velocity > 0.0f && velocity < 0.05f)
			ApplyLinearImpulse({ 0.0f,  m_GravityModifier });

		// Update jump animation frame
		uint16_t frame = velocity > 0.0f ? (m_JumpTimer < s_JumpFrameSwitchTime ? 0 : 1) : 2;
		animation.SetAnimationFrame(frame);
		m_State = Jump;
	}

	// Update animation and timer
	animation.PlayAnimation(m_State);
	animation.SetMirrorFlip(m_Direction < 0.0f);
	m_JumpTimer += ts;
}
