#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Editor/Panels/SceneViewportPanel.h"
#include "Proton/Editor/Panels/InspectorPanel.h"
#include "Proton/Editor/Panels/SceneHierarchyPanel.h"
#include "Proton/Editor/Panels/ToolbarPanel.h"
#include "Proton/Editor/Panels/ContentBrowserPanel.h"
#include "Proton/Editor/Panels/SettingsPanel.h"
#include "Proton/Editor/Panels/InfoPanel.h"

#include "Proton/Core/Application.h"
#include "Proton/Core/Window.h"
#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Graphics/Renderer/Framebuffer.h"
#include "Proton/Events/KeyEvents.h"
#include "Proton/Events/MouseEvents.h"
#include "Proton/Utils/Utils.h"
#include "Proton/Scene/SceneManager.h"
#include "Proton/Physics/PhysicsWorld.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.cpp>
#include <backends/imgui_impl_glfw.cpp>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace proton {

	struct EditorPanels
	{
		SettingsPanel Settings;
		InfoPanel Info;
		InspectorPanel Inspector;
		SceneHierarchyPanel SceneHiearchy;
		ToolbarPanel Toolbar;
		ContentBrowserPanel ContentBrowser;
		SceneViewportPanel SceneViewport;
	} static s_Panels;

	struct EditorFonts
	{
		ImFont* FontAwesome = nullptr;
		ImFont* SmallFont = nullptr;
	} static s_Fonts;

	EditorLayer* EditorLayer::s_Instance = nullptr;

	EditorLayer::EditorLayer()
	{
		EditorLayer::s_Instance = this;
	}

	void EditorLayer::OnCreate()
	{
		// ImGui setup
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		SetupFonts();
		SetupImGuiViewports();
		SetupThemeStyle();

		// Initialize ImGui implementation for GLFW
		InitializeImGui();

		// Store editor panels in vector
		m_EditorPanels.push_back(&s_Panels.Settings);
		m_EditorPanels.push_back(&s_Panels.Info);
		m_EditorPanels.push_back(&s_Panels.Inspector);
		m_EditorPanels.push_back(&s_Panels.SceneHiearchy);
		m_EditorPanels.push_back(&s_Panels.Toolbar);
		m_EditorPanels.push_back(&s_Panels.ContentBrowser);
		m_EditorPanels.push_back(&s_Panels.SceneViewport);

		for (EditorPanel* panel : m_EditorPanels)
			panel->OnCreate();
	}

	void EditorLayer::OnDestroy()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void EditorLayer::OnUpdate(float ts)
	{
		for (auto& panel : m_EditorPanels)
			panel->OnUpdate(ts);
	}

	void EditorLayer::OnImGuiRender()
	{
		//ImGui::ShowDemoWindow(); // Demo window for reference
		
		// DockSpace window flags
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize, 0.0f);

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		static bool dockspaceOpen = true;
		ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar(3);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
		}

		style.WindowMinSize.x = minWinSizeX;

		// Render editor panels
		m_MenuBar.OnImGuiRender();

		for (auto& panel : m_EditorPanels)
			panel->OnImGuiRender();

		ImGui::End(); // DockSpace
		ImGui::PopStyleVar();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		// Block events if viewport is not hovered by mouse
		if (m_BlockEvents && !s_Panels.SceneViewport.m_MoveEditorCamera)
		{
			event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			event.Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
			if (event.Handled)
				return;
		}
		else if (event.IsInCategory(EventCategoryKeyboard) && io.WantTextInput)
		{
			event.Handled = true;
			return;
		}

		for (auto& panel : m_EditorPanels)
			panel->OnEvent(event);
	}

	SceneViewportPanel* EditorLayer::GetSceneViewportPanel()
	{
		return &s_Panels.SceneViewport;
	}

	void EditorLayer::SetActiveScene(Scene* scene)
	{
		s_Instance->m_ActiveScene = scene;
		for (auto& panel : s_Instance->m_EditorPanels)
			panel->m_ActiveScene = scene;
		
		EditorLayer::SelectEntity({});
	}

	void EditorLayer::SelectEntity(Entity entity)
	{
		s_Instance->m_SelectedEntity = entity;
		for (auto& panel : s_Instance->m_EditorPanels)
			panel->m_SelectedEntity = entity;
	}

	void EditorLayer::OnBeginSceneSimulation(Scene* scene)
	{
		m_SceneBackup[scene->m_SceneFilepath] = scene->CreateSceneCopy();
	}

	void EditorLayer::OnStopSceneSimulation(Scene* scene)
	{
		bool isActiveScene = scene == m_ActiveScene;
		std::string sceneFilepath = scene->m_SceneFilepath;
		SceneManager::s_Instance->m_Scenes[sceneFilepath] = m_SceneBackup.at(sceneFilepath);
		if (isActiveScene)
			SceneManager::SetActiveScene(sceneFilepath);
		m_SceneBackup.erase(sceneFilepath);
	}

	EditorCamera* EditorLayer::GetCamera()
	{
		return s_Panels.SceneViewport.m_Camera.get();
	}

	ImFont* EditorLayer::GetFontAwesome()
	{
		return s_Fonts.FontAwesome;
	}

	ImFont* EditorLayer::GetSmallFont()
	{
		return s_Fonts.SmallFont;
	}

	void EditorLayer::SetupFonts()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		// Main font
		const EditorConfig::Font font = m_Config.EditorFonts["roboto"];
		io.Fonts->AddFontFromFileTTF(font.FontFilepath.c_str(), font.FontSize);
		// Small font
		s_Fonts.SmallFont = io.Fonts->AddFontFromFileTTF(font.FontFilepath.c_str(), 14.0f);
		// Font Awesome
		static const ImWchar icons_ranges[] = { 0xE000, 0xF8FF, 0 };
		s_Fonts.FontAwesome = io.Fonts->AddFontFromFileTTF("editor/content/font/FontAwesome.ttf", 18.0f, NULL, icons_ranges);
	}

	void EditorLayer::SetupThemeStyle()
	{
		// Styles
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiIO& io = ImGui::GetIO();

		style.FrameRounding = 7.0f;
		style.PopupRounding = 7.0f;
		style.ScrollbarSize = 20.0f;
		style.WindowBorderSize = 0.0f;

		// Color styles
		style.Colors[ImGuiCol_Border] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);

		style.Colors[ImGuiCol_Button] = ImVec4(0.4f, 0.18f, 0.19f, 1.0f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.18f, 0.19f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.6f, 0.18f, 0.19f, 1.0f);

		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.7f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

		style.Colors[ImGuiCol_Header] = ImVec4(0.32f, 0.32f, 0.32f, 1.0f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.0f);

		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.23f, 0.23f, 0.22f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.23f, 0.23f, 0.22f, 1.0f);

		style.Colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	}

	void EditorLayer::SetupImGuiViewports()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		io.ConfigFlags = ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
		// Enable viewports (ImGui windows can be detached from main application window)
		if (m_EnableViewports)
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		if (m_EnableViewports && io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
	}

	void EditorLayer::InitializeImGui()
	{
		auto window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void EditorLayer::BeginImGuiRender()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void EditorLayer::EndImGuiRender()
	{
		auto& io = ImGui::GetIO();
		auto& window = Application::Get().GetWindow();
		io.DisplaySize = { (float)window.GetWidth(), (float)window.GetHeight() };

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	#ifdef PROTON_PLATFORM_WINDOWS
		if (m_EnableViewports && io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	#endif
	}

}
#endif
