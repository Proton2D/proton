//
// Windows GLFW Window implementation
// From: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Platform/Windows/WindowsWindow.h
//
#pragma once

#include "Proton/Core/Window.h"

#include <GLFW/glfw3.h>

namespace proton {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const std::string& title, uint32_t width, uint32_t height);
		virtual ~WindowsWindow();

		virtual void OnUpdate() override;

		virtual unsigned int GetWidth() const override { return m_Data.Width; }
		virtual unsigned int GetHeight() const override { return m_Data.Height; }
		virtual inline float GetAspectRatio() const override { return (float)GetWidth() / (float)GetHeight(); }

		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool enabled) override;
		virtual bool IsVSync() const override;

		virtual void SetFullscreen(bool fullscreen = true) override;
		virtual bool IsFullscreen() const override;

		virtual void* GetNativeWindow() const { return m_Window; }

	private:
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;
		// For disabling fullscreen and restoring previous width and height
		unsigned int m_PreviousWidth, m_PreviousHeight;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			bool Fullscreen;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}
