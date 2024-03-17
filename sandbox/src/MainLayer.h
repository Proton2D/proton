#pragma once

class MainLayer : public proton::AppLayer
{
public:
	// Caution: Methods are executed even if scene is not simulated
	virtual void OnCreate() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnEvent(proton::Event& e);

private:
	void SpawnRandomBox(const glm::vec2& position);
};
