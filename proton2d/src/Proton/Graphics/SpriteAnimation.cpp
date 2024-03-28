#include "ptpch.h"
#include "Proton/Graphics/SpriteAnimation.h"
#include "Proton/Scene/Entity.h"

namespace proton {

    void SpriteAnimation::AddAnimation(uint16_t index, uint16_t frameCount, AnimationPlayMode playmode)
    {
        m_Animations[index] = Animation{ frameCount, playmode };
        if (m_CurrentAnimationIndex == -1)
            PlayAnimation(index);
    }

    void SpriteAnimation::PlayAnimation(uint16_t index, uint16_t startFrame)
    {
        PT_CORE_ASSERT(m_Animations.find(index) != m_Animations.end(), "Animation not found");
        if (index != m_CurrentAnimationIndex)
        {
            auto& sprite = m_OwningEntity->GetSprite();
            m_CurrentAnimationIndex = index;
            m_CurrentAnimation = &m_Animations[index];
            sprite.SetTile(startFrame, index);
        }
    }

    void SpriteAnimation::SetAnimationFrame(uint16_t frame)
    {
        auto& sprite = m_OwningEntity->GetSprite();
        sprite.SetTile(frame, m_CurrentAnimationIndex);
        m_CurrentFrame = frame;
    }

    void SpriteAnimation::SetMirrorFlip(bool mirror_x, bool mirror_y)
    {
        auto& sprite = m_OwningEntity->GetSprite();
        sprite.MirrorFlip(mirror_x, mirror_y);
    }

    void SpriteAnimation::Replay()
    {
        SetAnimationFrame(0);
    }

    float SpriteAnimation::GetProgress()
    {
        if (!m_CurrentAnimation->FrameCount)
            return 0.0f;
        return (float)m_CurrentFrame / m_CurrentAnimation->FrameCount;
    }

    void SpriteAnimation::Update(float ts)
    {
        if (!m_CurrentAnimation->FrameCount)
            return;

        if (m_CurrentAnimation->PlayMode == AnimationPlayMode::PAUSED)
            return;

        m_ElapsedTime += ts;

        if (m_ElapsedTime >= m_FrameTime)
        {
            auto& sprite = m_OwningEntity->GetSprite();
            if (m_CurrentAnimation->PlayMode == AnimationPlayMode::REPEAT)
            {
                m_CurrentFrame %= m_CurrentAnimation->FrameCount;
                sprite.SetTile(m_CurrentFrame, sprite.GetTilePos().y);
            }
            else // if (m_PlayMode == AnimationPlayMode::PLAY_ONCE)
            {
                if (m_CurrentFrame < m_CurrentAnimation->FrameCount)
                    sprite.SetTile(m_CurrentFrame, sprite.GetTilePos().y);
                else
                    sprite.SetTile(0, sprite.GetTilePos().y);
            }

            m_ElapsedTime = 0.0f;
            m_CurrentFrame++;
        }
    }

    bool SpriteAnimation::FinishedPlaying()
    {
        return m_CurrentFrame == m_CurrentAnimation->FrameCount;
    }

    void SpriteAnimation::SetFPS(uint16_t fps)
    {
        m_FPS = fps;
        m_FrameTime = 1.0f / fps;
    }

}
