#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/EditorLayer.h"

#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Graphics/Renderer/Framebuffer.h"
#include "Proton/Core/Application.h"
#include "Proton/Core/Window.h"
#include "Proton/Utils/Utils.h"
#include "Proton/Events/MouseEvents.h"
#include "Proton/Events/KeyEvents.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.cpp>
#include <backends/imgui_impl_glfw.cpp>

#include <imgui.h>

namespace proton {

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

		// Font
		auto& io = ImGui::GetIO();
		const EditorConfig::Font font = m_Config.EditorFonts["roboto"];
		io.Fonts->AddFontFromFileTTF(font.FontFilepath.c_str(), font.FontSize);
		
		io.ConfigFlags = ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
		// Enable viewports (ImGui windows can be detached from main application window)
		if (m_EnableViewports)
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Styles
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 7.0f;
		style.PopupRounding = 7.0f;
		style.ScrollbarSize = 20.0f;
		style.WindowBorderSize = 0.0f;

		// Theme styles
		// TODO: Move values to editor config file
		style.Colors[ImGuiCol_Border] = ImVec4(0.28f, 0.12f, 0.12f, 1.0f);;
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.28f, 0.12f, 0.12f, 1.0f);

		style.Colors[ImGuiCol_Button] = ImVec4(0.5f, 0.12f, 0.12f, 1.0f);;
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.6f, 0.12f, 0.12f, 1.0f);;
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7f, 0.12f, 0.12f, 1.0f);;

		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.7f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

		ImVec4 headerColor = ImVec4(0.5f, 0.12f, 0.12f, 1.0f);
		style.Colors[ImGuiCol_Header] = headerColor;
		style.Colors[ImGuiCol_HeaderHovered] = headerColor;
		style.Colors[ImGuiCol_HeaderActive] = headerColor;

		ImVec4 titleColor = ImVec4(0.32f, 0.12f, 0.12f, 1.0f);
		style.Colors[ImGuiCol_TitleBg] = titleColor;
		style.Colors[ImGuiCol_TitleBgActive] = titleColor;

		ImVec4 tabColor = ImVec4(0.5f, 0.2f, 0.1f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = tabColor;
		style.Colors[ImGuiCol_TabHovered] = tabColor;
		style.Colors[ImGuiCol_TabActive] = tabColor;

		if (m_EnableViewports && io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Initialize ImGui
		auto window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Check if cache directory exist
		if (!std::filesystem::exists("editor/cache/"))
			if (std::filesystem::create_directories("editor/cache/"))
				PT_CORE_ERROR("[EditorLayer::OnCreate] Could not create cache directory!");
	
		m_MiscPanel.m_SceneViewportPanel = &m_SceneViewportPanel;
		m_EditorPanels.push_back(&m_SceneViewportPanel);
		m_EditorPanels.push_back(&m_SceneHiearchyPanel);
		m_EditorPanels.push_back(&m_InspectorPanel);
		m_EditorPanels.push_back(&m_MiscPanel);
		m_EditorPanels.push_back(&m_ScenePanel);
		m_EditorPanels.push_back(&m_PrefabPanel);
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

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		static bool dockspaceOpen = true;
		ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

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
	}

	void EditorLayer::OnEvent(Event& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		// Block events if viewport is not hovered by mouse
		if (m_BlockEvents && !m_SceneViewportPanel.m_MoveEditorCamera)
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

	void EditorLayer::SetActiveScene(Scene* scene)
	{
		s_Instance->m_ActiveScene = scene;
		for (auto& panel : s_Instance->m_EditorPanels)
			panel->m_ActiveScene = scene;
	}

	void EditorLayer::SelectEntity(Entity entity)
	{
		s_Instance->m_SelectedEntity = entity;
		for (auto& panel : s_Instance->m_EditorPanels)
			panel->m_SelectedEntity = entity;
	}

	EditorCamera& EditorLayer::GetCamera() 
	{
		return s_Instance->m_SceneViewportPanel.m_Camera; 
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
