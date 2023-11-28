#pragma once
#include "ptpch.h"
#include "Proton/Events/Event.h"
#include "Proton/Core/KeyCodes.h"

namespace proton {

	// Virtual class for key events
	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }

	protected:
		KeyEvent(const KeyCode keycode) : m_KeyCode(keycode) {}

		KeyCode m_KeyCode;
	};


	class KeyPressedEvent : public KeyEvent 
	{
	public:
		EVENT_CLASS_TYPE(KeyPressed)
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard)

		KeyPressedEvent(const KeyCode keycode, const uint16_t repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount) {}

		uint16_t GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}
		
	private:
		uint16_t m_RepeatCount;
	};


	class KeyReleasedEvent : public KeyEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyReleased)
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard)

		KeyReleasedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}
	};


	class KeyTypedEvent : public KeyEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyTyped)
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard)

		KeyTypedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}
	};

}
