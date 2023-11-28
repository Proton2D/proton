#pragma once

#include "Proton/Events/Event.h"
#include "Proton/Core/KeyCodes.h"

namespace proton {

	class MouseMovedEvent : public Event
	{
	public:
		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse)

		MouseMovedEvent(const float x, const float y)
			: m_MouseX(x), m_MouseY(y) {}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

	private:
		float m_MouseX, m_MouseY;
	};


	class MouseScrolledEvent : public Event
	{
	public:
		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse)

		MouseScrolledEvent(const float xOffset, const float yOffset)
			: m_xOffset(xOffset), m_yOffset(yOffset) {}

		float GetXOffset() const { return m_xOffset; }
		float GetYOffset() const { return m_yOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

	private:
		float m_xOffset, m_yOffset;
	};


	// Virtual class for mouse buttons events
	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }

	protected:
		MouseButtonEvent(const MouseCode button)
			: m_Button(button) {}

		MouseCode m_Button;
	};


	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseButtonPressed)
		EVENT_CLASS_CATEGORY(EventCategoryMouse)

		MouseButtonPressedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}
	};


	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseButtonReleased)
		EVENT_CLASS_CATEGORY(EventCategoryMouse)

		MouseButtonReleasedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}
	};

}
