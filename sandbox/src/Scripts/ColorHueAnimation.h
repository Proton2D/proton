#pragma once

class ColorHueAnimation : public EntityScript
{
public:
	ENTITY_SCRIPT_CLASS(ColorHueAnimation)

	virtual void OnRegisterFields() override
	{
		RegisterField(ScriptFieldType::Bool, "Back and Forth", &m_BackAndForth);
		RegisterField(ScriptFieldType::Float, "Speed", &m_Speed);
		RegisterField(ScriptFieldType::Float, "Saturation", &m_Saturation);
		RegisterField(ScriptFieldType::Float, "HueRangeP", &m_HueRangeP, false);
		RegisterField(ScriptFieldType::Float, "HueRangeQ", &m_HueRangeQ, false);
	}

	virtual bool OnCreate() override
	{
		if (HasComponent<SpriteComponent>())
			m_Sprite = &GetComponent<SpriteComponent>();

		if (HasComponent<ResizableSpriteComponent>())
			m_ResizableSprite = &GetComponent<ResizableSpriteComponent>();

		return true;
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Sprite)
			UpdateHue(ts, m_Sprite->Color);

		if (m_ResizableSprite)
			UpdateHue(ts, m_ResizableSprite->Color);
	}

	void UpdateHue(float ts, glm::vec4& colorRGBA)
	{
		glm::vec4 colorHSV = Utils::Graphics::RGBAtoHSV(colorRGBA);
		float hue = glm::clamp(colorHSV[0] + m_Speed * m_Direction * ts, m_HueRangeP, m_HueRangeQ);
		if (hue == m_HueRangeQ)
		{
			if (m_BackAndForth)
			{
				m_Direction = -1.0f;
				hue -= 0.001f;
			}
			else
				hue = m_HueRangeP;
		}
		else if (m_BackAndForth && hue == m_HueRangeP)
			m_Direction = 1.0f;

		colorHSV[0] = hue;
		colorHSV[1] = m_Saturation;
		colorRGBA = Utils::Graphics::HSVtoRGBA(colorHSV);
	}

	virtual void OnImGuiRender() override
	{
	#ifdef PT_EDITOR
		ImGui::DragFloatRange2("HueRange", &m_HueRangeP, &m_HueRangeQ, 0.001f, 0.0f, 1.0f);
	#endif
	}

private:
	bool m_BackAndForth = false;
	float m_Speed = 0.5f;
	float m_Saturation = 0.8f;
	float m_HueRangeP = 0.0f;
	float m_HueRangeQ = 1.0f;
	
	float m_Direction = 1.0f;

	SpriteComponent* m_Sprite = nullptr;
	ResizableSpriteComponent* m_ResizableSprite = nullptr;
};
