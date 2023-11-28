#pragma once

#include "Proton/Core/Base.h"
#include "Proton/Events/Event.h"

namespace proton {

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual inline float GetAspectRatio() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetFullscreen(bool fullscreen) = 0;
		virtual bool IsFullscreen() const = 0;

		virtual void* GetNativeWindow() const = 0;
	};

}
