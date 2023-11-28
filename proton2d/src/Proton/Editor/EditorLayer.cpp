#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Editor/Panels/MiscellaneousPanel.h"
#include "Proton/Editor/Panels/InspectorPanel.h"
#include "Proton/Editor/Panels/SceneHierarchyPanel.h"
#include "Proton/Editor/Panels/ScenePanel.h"
#include "Proton/Editor/Panels/PrefabPanel.h"

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
		style.WindowMenuButtonPosition = ImGuiDir_None;
		style.ColorButtonPosition;

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

		// Initialize editor panels
		PushEditorPanel("MiscellaneousPanel", new MiscellaneousPanel());
		PushEditorPanel("InspectorPanel", new InspectorPanel());
		PushEditorPanel("SceneHierarchy", new SceneHierarchyPanel());
		PushEditorPanel("ScenePanel", new ScenePanel());
		PushEditorPanel("PrefabPanel", new PrefabPanel());

		// Check if cache directory exist
		if (!std::filesystem::exists("editor/cache/"))
			if (std::filesystem::create_directories("editor/cache/"))
				PT_CORE_ERROR("[EditorLayer::OnCreate] Could not create cache directory!");

		// Create framebuffer for scene viewport
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = MakeShared<Framebuffer>(fbSpec);
	}

	void EditorLayer::OnDestroy()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		for (auto& p : m_EditorPanels)
			delete p.second;
	}

	void EditorLayer::OnUpdate(float ts)
	{
		// On viewport resize
		FramebufferSpecification spec = m_Framebuffer->GetSpecification();
		if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_Camera.OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
			Renderer::SetViewport(0, 0, (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_Framebuffer->Bind();
		Renderer::SetClearColor(m_ActiveScene->m_ClearColor);
		Renderer::Clear();

		m_ActiveScene->OnUpdate(ts * Application::Get().GetTimeScale());

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		m_MousePos = { (int)mx, (int)my };

		DrawCollidersAndSelectionOutline();
		m_Framebuffer->Unbind();

		const glm::vec2& cursor = m_ActiveScene->GetCursorWorldPosition();

		// Update editor camera
		m_Camera.OnUpdate(ts);

		// Move selected entity
		if (m_MoveSelectedEntity && m_SelectedEntity.IsValid())
		{
			glm::vec2 targetPos = cursor + m_SelectionMouseOffset;
			auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
			transform.Position.x = targetPos.x;
			transform.Position.y = targetPos.y;
			ImGui::SetMouseCursor(7);
		}

		// Move editor camera
		if (m_MoveEditorCamera)
		{
			glm::vec2 offset = m_CameraDragOffset - cursor;
			m_Camera.m_Position.x += offset.x;
			m_Camera.m_Position.y += offset.y;
			ImGui::SetMouseCursor(2);
		}

		// Update editor panels
		for (auto& panel : m_EditorPanels)
			panel.second->OnUpdate(ts);
	}

	void EditorLayer::OnImGuiRender()
	{
		//ImGui::ShowDemoWindow(); // Demo window for reference

		// Viewport and dockspace implementation from Hazel
		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		// Scene viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

		ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Begin("Viewport");
		ImGui::PopStyleColor();
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		m_BlockEvents = !m_ViewportHovered;

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		ImGui::End();
		ImGui::PopStyleVar();

		// Render editor components
		m_MenuBar.OnImGuiRender();
		for (auto& panel : m_EditorPanels) {
			panel.second->OnImGuiRender();
		}

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		// Block events if viewport is not hovered by mouse
		if (m_BlockEvents && !m_MoveEditorCamera)
		{
			event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			event.Handled |= event.IsInCategory(EventCategoryKeyboard) & (io.WantCaptureKeyboard || io.WantTextInput);
			if (event.Handled)
				return;
		}
		else if (event.IsInCategory(EventCategoryKeyboard) && io.WantTextInput)
		{
			event.Handled = true;
			return;
		}

		m_Camera.OnEvent(event);
		const glm::vec2& cursor = m_ActiveScene->GetCursorWorldPosition();
		
		EventDispatcher dispatcher(event);

		// Dispatch mouse events
		dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
		{
			SceneState state = m_ActiveScene->GetSceneState();

			// Mouse Button 1 (Right): Move editor camera
			if (e.GetMouseButton() == Mouse::Button1 && !m_MoveEditorCamera 
				&& (state == SceneState::Stop || m_UseEditorCameraInRuntime))
			{
				m_CameraDragOffset = cursor;
				m_MoveEditorCamera = true;
			}

			// Mouse Button 0 (Left): Select Entity
			else if (e.GetMouseButton() == Mouse::Button0)
			{
				Entity target; float transformMaxZ = 0.0f;

				for (auto& entity : m_ActiveScene->GetEntitiesOnCursorLocation())
				{
					auto& transform = entity.GetComponent<TransformComponent>();
					if (!target || transform.Position.z > transformMaxZ)
					{
						target = entity;
						transformMaxZ = transform.Position.z;
					}
				}

				if (m_SelectedEntity && m_ActiveScene->IsCursorHoveringEntity(m_SelectedEntity))
				{
					auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
					auto& targetTransform = target.GetComponent<TransformComponent>();
					if (transform.Scale.x < targetTransform.Scale.x
						&& transform.Scale.y < targetTransform.Scale.y)
					{
						// Discard selection
						target = m_SelectedEntity;
					}	
				}

				if (target && target == m_SelectedEntity)
				{
					m_MoveSelectedEntity = true;
					auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
					m_SelectionMouseOffset = glm::vec2{ transform.Position.x, transform.Position.y } - cursor;
				}

				SelectEntity(target);
			}

			return false;
		});

		// Dispatch keyboard events
		dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e)
		{
			KeyCode key = e.GetKeyCode();

			if (key == Key::F1)
				m_ShowSelectionOutline = !m_ShowSelectionOutline;
			
			if (key == Key::F2)
				m_ShowSelectionCollider = !m_ShowSelectionCollider;
			
			if (key == Key::F3)
				m_ShowAllColliders = !m_ShowAllColliders;

			if (key == Key::Escape && m_SelectedEntity)
				SelectEntity({});

			if (key == Key::Delete && m_SelectedEntity)
			{
				m_SelectedEntity.Destroy();
				SelectEntity({});
			}

			return false;
		});

		// Mouse evenets (button released)
		dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e)
		{
			if (e.GetMouseButton() == Mouse::Button0)
				m_MoveSelectedEntity = false;
			if (e.GetMouseButton() == Mouse::Button1)
				m_MoveEditorCamera = false;

			ImGui::SetMouseCursor(0);

			return false;
		});
	}

	void EditorLayer::DrawCollidersAndSelectionOutline()
	{
		Renderer::BeginScene(*m_ActiveScene->GetPrimaryCamera(), m_ActiveScene->GetPrimaryCameraPosition());
		// Draw box colliders
		auto view = m_ActiveScene->m_Registry.view<TransformComponent, BoxColliderComponent>();
		for (auto entity : view)
		{
			auto [transform, bc] = view.get<TransformComponent, BoxColliderComponent>(entity);

			// Check if current entity is selected and draw collider rect
			bool drawSelected = m_ShowSelectionCollider && m_SelectedEntity.m_Handle == entity;

			if (m_ShowAllColliders || drawSelected)
			{
				float zPos = (m_ShowAllColliders && drawSelected) ? 0.205f : 0.2f;
				glm::vec4 color = (m_ShowAllColliders && drawSelected) 
					? glm::vec4{ 0.9f, 0.3f, 0.3f, 0.5f } : glm::vec4{ 0.9f, 0.6f, 0.3f, 0.5f };
				glm::vec3 position = { transform.Position.x + bc.Offset.x, transform.Position.y + bc.Offset.y, zPos };
				glm::vec3 scale    = { bc.Size.x * transform.Scale.x, bc.Size.y * transform.Scale.y, 1.0f };
				glm::mat4 transformMatrix = Math::GetTransform(position, scale, transform.Rotation);

				Renderer::DrawQuad(transformMatrix, color);
			}
		}

		// Draw selected entity outline
		if (m_SelectedEntity.IsValid() && m_ShowSelectionOutline)
		{
			auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
			float padding = glm::sqrt(m_ActiveScene->GetPrimaryCamera()->GetZoomLevel()) * 0.05f;
			glm::vec3 position = { transform.Position.x, transform.Position.y, 0.21f };
			glm::vec3 scale    = { transform.Scale.x + padding, transform.Scale.y + padding, 1.0f };
			glm::mat4 transformMatrix = Math::GetTransform(position, scale, transform.Rotation);

			glm::vec4 color = m_ShowSelectionOutline && m_MoveSelectedEntity
				? glm::vec4{ 0.8f, 0.8f, 0.2f, 1.0f } : glm::vec4{ 1.0f };

			if (m_SelectedEntity.HasComponent<SpriteComponent>())
			{
				auto& sprite = m_SelectedEntity.GetComponent<SpriteComponent>();
				if (sprite.Sprite)
					scale.x *= (float)sprite.Sprite.m_PixelSize.x / (float)sprite.Sprite.m_PixelSize.y;
			}
			Renderer::SetLineWidth(glm::min(50.0f * padding, 1.0f));
			Renderer::DrawRect(transformMatrix, color);
		}
		Renderer::EndScene();
	}

	void EditorLayer::ResetCameraPosition()
	{
		m_Camera.m_Position = { 0.0f, 0.0f, 0.0f };
	}

	void EditorLayer::SetActiveScene(Scene* scene)
	{
		s_Instance->m_ActiveScene = scene;
		for (auto& panel : s_Instance->m_EditorPanels)
		{
			panel.second->m_ActiveScene = scene;
		}
	}

	void EditorLayer::SelectEntity(Entity entity)
	{
		s_Instance->m_SelectedEntity = entity;
		for (auto& panel : s_Instance->m_EditorPanels)
			panel.second->m_SelectedEntity = entity;
	}

	void EditorLayer::PushEditorPanel(const std::string& name, EditorPanel* panel)
	{
		m_EditorPanels.push_back(std::make_pair(name, panel));
		panel->m_ActiveScene = m_ActiveScene;
	}

	void EditorLayer::PopEditorPanel(const std::string& name)
	{
		m_EditorPanels.erase(std::remove_if(m_EditorPanels.begin(), m_EditorPanels.end(),
			[&name](const std::pair<std::string, EditorPanel*>& element) {
				return element.first == name;
			}),
			m_EditorPanels.end());
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
