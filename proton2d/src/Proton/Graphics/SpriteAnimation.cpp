#include "ptpch.h"
#include "Proton/Graphics/SpriteAnimation.h"

namespace proton {

    SpriteAnimation::SpriteAnimation(Sprite* sprite)
        : m_Sprite(sprite)
    {
    }

    void SpriteAnimation::SetSprite(Sprite* sprite)
    {
        m_Sprite = sprite;
    }

    void SpriteAnimation::AddAnimation(uint16_t index, uint16_t frameCount)
    {
        m_AnimationsFrameCount[index] = frameCount;
    }

    void SpriteAnimation::StartAnimation(uint16_t index, bool mirror_x, bool mirror_y)
    {
        if (index != m_CurrentAnimationIndex)
        {
            m_CurrentAnimationIndex = index;
            m_CurrentAnimationFrameCount = m_AnimationsFrameCount[index];
            m_Sprite->SetTile(0, index);
        }
        SetMirrorFlip(mirror_x, mirror_y);
    }

    void SpriteAnimation::SetAnimation(uint16_t index, bool mirror_x, bool mirror_y)
    {
        PT_CORE_ASSERT(m_AnimationsFrameCount.find(index) != m_AnimationsFrameCount.end(), "Animation not found");
        if (index != m_CurrentAnimationIndex)
        {
            m_CurrentAnimationIndex = index;
            m_CurrentAnimationFrameCount = m_AnimationsFrameCount[index];
            m_CurrentFrame %= m_CurrentAnimationFrameCount;
            m_Sprite->SetTile(m_CurrentFrame, index);
        }
        SetMirrorFlip(mirror_x, mirror_y);
    }

    void SpriteAnimation::SetAnimationFrame(uint16_t frame)
    {
        m_Sprite->SetTile(frame, m_CurrentAnimationIndex);
        m_CurrentFrame = frame;
    }

    void SpriteAnimation::SetMirrorFlip(bool mirror_x, bool mirror_y)
    {
        m_Sprite->MirrorFlip(mirror_x, mirror_y);
    }

    void SpriteAnimation::SetPlayMode(AnimationPlayMode mode, uint16_t startFrame, bool restartAnimation)
    {
        m_PlayMode = mode;
        if (restartAnimation)
            SetAnimationFrame(startFrame);
    }

    void SpriteAnimation::Replay()
    {
        SetAnimationFrame(0);
    }

    float SpriteAnimation::GetProgress()
    {
        if (!m_CurrentAnimationFrameCount)
            return 0.0f;
        return (float)m_CurrentFrame / m_CurrentAnimationFrameCount;
    }

    void SpriteAnimation::Update(float ts)
    {
        if (!m_CurrentAnimationFrameCount)
            return;

        if (m_PlayMode == AnimationPlayMode::PAUSED)
            return;

        m_ElapsedTime += ts;

        if (m_ElapsedTime >= m_FrameTime)
        {
            if (m_PlayMode == AnimationPlayMode::REPEAT)
            {
                m_CurrentFrame %= m_CurrentAnimationFrameCount;
                m_Sprite->SetTile(m_CurrentFrame, m_Sprite->GetTilePos().y);
            }
            else // if (m_PlayMode == AnimationPlayMode::PLAY_ONCE)
            {
                if (m_CurrentFrame < m_CurrentAnimationFrameCount)
                    m_Sprite->SetTile(m_CurrentFrame, m_Sprite->GetTilePos().y);
                else
                    m_Sprite->SetTile(0, m_Sprite->GetTilePos().y);
            }

            m_ElapsedTime = 0.0f;
            m_CurrentFrame++;
        }
    }

    bool SpriteAnimation::FinishedPlaying()
    {
        return m_CurrentFrame == m_CurrentAnimationFrameCount;
    }

    void SpriteAnimation::SetFPS(uint16_t fps)
    {
        m_FPS = fps;
        m_FrameTime = 1.0f / fps;
    }

}
