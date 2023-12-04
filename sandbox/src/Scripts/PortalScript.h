#pragma once

class PortalScript : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(PortalScript)

	virtual bool OnCreate() override;
	virtual void OnRegisterFields() override;
private:
	SpriteAnimation* m_SpriteAnimation;
	int m_TargetLevel = 1;
};
