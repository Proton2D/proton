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
            m_CurrentAnimationIndex = index;
            m_CurrentAnimation = &m_Animations[index];
            m_Sprite->SetTile(startFrame, index);
        }
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
            if (m_CurrentAnimation->PlayMode == AnimationPlayMode::REPEAT)
            {
                m_CurrentFrame %= m_CurrentAnimation->FrameCount;
                m_Sprite->SetTile(m_CurrentFrame, m_Sprite->GetTilePos().y);
            }
            else // if (m_PlayMode == AnimationPlayMode::PLAY_ONCE)
            {
                if (m_CurrentFrame < m_CurrentAnimation->FrameCount)
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
        return m_CurrentFrame == m_CurrentAnimation->FrameCount;
    }

    void SpriteAnimation::SetFPS(uint16_t fps)
    {
        m_FPS = fps;
        m_FrameTime = 1.0f / fps;
    }

}
