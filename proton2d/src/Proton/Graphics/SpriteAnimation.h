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
		void AddAnimation(uint16_t index, uint16_t frameCount, AnimationPlayMode playmode = AnimationPlayMode::REPEAT);
		// index - spritesheet Y tile pos (from image bottom)
		void PlayAnimation(uint16_t index, uint16_t startFrame = 0);
		
		void SetAnimationFrame(uint16_t frame);
		void SetMirrorFlip(bool mirror_x = false, bool mirror_y = false);
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
		
		struct Animation
		{
			uint16_t FrameCount;
			AnimationPlayMode PlayMode;
		};

		std::unordered_map<uint16_t, Animation> m_Animations;
		uint16_t m_CurrentAnimationIndex = -1;
		Animation* m_CurrentAnimation = nullptr;
		//uint16_t m_CurrentAnimationFrameCount = 0;

		uint16_t m_FPS = 60;
		uint16_t m_CurrentFrame = 0;

		float m_FrameTime = 1.0f / m_FPS;
		float m_ElapsedTime = 0.0f;

		friend class Scene;
		friend class Entity;
	};
}
