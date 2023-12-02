#pragma once

class ParallaxBackground : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(ParallaxBackground)

	virtual void OnRegisterFields() override;
	virtual bool OnCreate() override;
	virtual void OnUpdate(float ts) override;
private:
	float m_ParallaxFactor = 1.0f;
	float m_PositionOffset = 0.0f;

	Scene* m_Scene;
	std::vector<Entity> m_Copies;
	float m_SpriteAspectRatio;
	uint32_t m_CopiesCount;
};
