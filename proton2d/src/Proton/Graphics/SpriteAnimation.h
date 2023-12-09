#pragma once

#include "Proton/Graphics/Sprite.h"

#include <unordered_map>

namespace proton {

	enum class AnimationPlayMode
	{
		REPEAT, PLAY_ONCE, PAUSED
	};

	class SpriteAnimation
	{
	public:
		SpriteAnimation() = default;
		SpriteAnimation(Sprite* sprite);

		// index - spritesheet Y tile pos (from image bottom)
		void AddAnimation(uint16_t index, uint16_t frameCount);
		// index - spritesheet Y tile pos (from image bottom)
		void StartAnimation(uint16_t index, bool mirror_x = false, bool mirror_y = false);
		void SetAnimation(uint16_t index, bool mirror_x = false, bool mirror_y = false);
		void SetAnimationFrame(uint16_t frame);
		void SetMirrorFlip(bool mirror_x = false, bool mirror_y = false);
		void SetPlayMode(AnimationPlayMode mode, uint16_t startFrame = 0, bool restartAnimation = true);
		void Replay();

		float GetProgress();
		// Used for AnimationPlayMode::PLAY_ONCE
		bool FinishedPlaying();

		void SetFPS(uint16_t fps);
		uint16_t GetFPS() const { return m_FPS; }

	private:
		void Update(float ts);
		void SetSprite(Sprite* sprite);

		Sprite* m_Sprite = nullptr;
		
		std::unordered_map<uint16_t, uint16_t> m_AnimationsFrameCount;
		uint16_t m_CurrentAnimationIndex = -1;
		uint16_t m_CurrentAnimationFrameCount = 0;

		AnimationPlayMode m_PlayMode = AnimationPlayMode::REPEAT;

		uint16_t m_FPS = 60;
		uint16_t m_CurrentFrame = 0;

		float m_FrameTime = 1.0f / m_FPS;
		float m_ElapsedTime = 0.0f;

		friend class Scene;
		friend class Entity;
	};
}
