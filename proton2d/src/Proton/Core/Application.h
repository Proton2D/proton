#pragma once

#include "Proton/Core/Base.h"
#include "Proton/Core/Window.h"
#include "Proton/Core/Config.h"
#include "Proton/Core/AppLayer.h"

#ifdef PT_EDITOR
#include "Proton/Editor/EditorLayer.h"
#endif

namespace proton {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void PushLayer(AppLayer* layer);
		void PushOverlay(AppLayer* layer);
		void Exit();

		inline float GetTimeScale() const { return m_TimeScale; };

		Window& GetWindow() { return *m_Window; }
		static Application& Get() { return *s_Instance; }

	protected:
		virtual bool OnCreate() = 0; // To be defined by client

		void OnEvent(Event& event);

	private:
		static Application* s_Instance;

		ApplicationConfig m_AppConfig;
		std::vector<AppLayer*> m_AppLayers;
		Unique<Window> m_Window;

		bool m_IsRunning = false;
		bool m_WindowMinimized = false;
		float m_FrameTime = 0.0f;
		float m_TimeScale = 1.0f;

	#ifdef PT_EDITOR
		EditorLayer* m_EditorLayer;
	#endif

		friend class SettingsPanel;
		friend class InfoPanel;
	};

}

// Application Entry Point
#ifdef PROTON_DISTRIBUTION

	#ifdef PROTON_PLATFORM_WINDOWS
		#include <Windows.h>
		#define PROTON_APPLICATION_ENTRY_POINT(ApplcationClass)\
		int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)\
		{\
			proton::Logger::Init();\
			ApplcationClass app;\
			app.Run();\
		}
	#endif

#else
	#define PROTON_APPLICATION_ENTRY_POINT(ApplcationClass)\
	int main(int argc, char** argv)\
	{\
		proton::Logger::Init();\
		ApplcationClass app;\
		app.Run();\
	}
#endif
