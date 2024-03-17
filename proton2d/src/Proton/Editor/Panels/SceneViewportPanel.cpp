#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/SceneViewportPanel.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Editor/EditorCamera.h"
#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Core/Application.h"
#include "Proton/Events/KeyEvents.h"
#include "Proton/Events/MouseEvents.h"
#include "Proton/Core/Input.h"
#include "Proton/Utils/Utils.h"
#include "Proton/Scene/SceneManager.h"
#include "Proton/Scene/PrefabManager.h"

#include <imgui.h>
#include <filesystem> // remove

namespace proton {

	void SceneViewportPanel::OnCreate()
	{
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = MakeShared<Framebuffer>(fbSpec);
		m_Camera = MakeUnique<EditorCamera>();
	}

	void SceneViewportPanel::OnImGuiRender()
	{
		// Scene Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		EditorLayer::Get()->m_BlockEvents = !m_ViewportHovered;

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		if (m_ActiveScene)
		{
			uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		}
		else
		{
			ImGui::Dummy(ImVec2{ m_ViewportSize.x, m_ViewportSize.y });
		}
 
		HandleImGuiDragAndDrop();

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneViewportPanel::OnUpdate(float ts)
	{
		if (!m_ActiveScene)
			return;

		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		// On viewport resize
		FramebufferSpecification spec = m_Framebuffer->GetSpecification();
		if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_Camera->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
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
		m_Camera->OnUpdate(ts);

		// Move selected entity
		if (m_MoveSelectedEntity && m_SelectedEntity.IsValid())
		{
			glm::vec2 targetPos = cursor + m_SelectionMouseOffset;
			auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
			transform.LocalPosition.y += targetPos.y - transform.WorldPosition.y;
			transform.LocalPosition.x += targetPos.x - transform.WorldPosition.x;
			ImGui::SetMouseCursor(7);
		}

		// Move editor camera
		if (m_MoveEditorCamera)
		{
			glm::vec2 offset = m_CameraDragOffset - cursor;
			m_Camera->m_Position.x += offset.x;
			m_Camera->m_Position.y += offset.y;
			ImGui::SetMouseCursor(2);
		}
	}

	void SceneViewportPanel::OnEvent(Event& event)
	{
		if (!m_ActiveScene)
			return;

		m_Camera->OnEvent(event);
		const glm::vec2& cursor = m_ActiveScene->GetCursorWorldPosition();

		EventDispatcher dispatcher(event);

		// Dispatch mouse events
		dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
		{
			SceneState state = m_ActiveScene->GetSceneState();

			// Mouse Button 1 (Right): Move editor camera
			if (e.GetMouseButton() == Mouse::Button1 && !m_MoveEditorCamera
				&& (state == SceneState::Stop || m_Camera->m_UseInRuntime))
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
					if (!entity.HasAnyComponent<SpriteComponent, ResizableSpriteComponent, CircleRendererComponent>())
						continue;

					auto& transform = entity.GetComponent<TransformComponent>();
					if (!target || transform.WorldPosition.z > transformMaxZ)
					{
						target = entity;
						transformMaxZ = transform.WorldPosition.z;
					}
				}

				if (m_SelectedEntity && m_ActiveScene->IsCursorHoveringEntity(m_SelectedEntity))
				{
					// Discard selection
					target = m_SelectedEntity;
				}

				if (target && target == m_SelectedEntity)
				{
					m_MoveSelectedEntity = true;
					auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
					m_SelectionMouseOffset = glm::vec2{ transform.WorldPosition.x, transform.WorldPosition.y } - cursor;
				}

				EditorLayer::Get()->SelectEntity(target);
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
				EditorLayer::Get()->SelectEntity({});

			if (key == Key::Delete && m_SelectedEntity)
			{
				m_SelectedEntity.Destroy();
				// TODO: remove
				EditorLayer::Get()->SelectEntity({});
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

	void SceneViewportPanel::DrawCollidersAndSelectionOutline()
	{
		Renderer::BeginScene(m_ActiveScene->GetPrimaryCamera(), m_ActiveScene->GetPrimaryCameraPosition());

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
				glm::vec3 position = { transform.WorldPosition.x + bc.Offset.x, transform.WorldPosition.y + bc.Offset.y, zPos };
				glm::vec3 scale = { bc.Size.x * transform.Scale.x, bc.Size.y * transform.Scale.y, 1.0f };
				glm::mat4 transformMatrix = Math::GetTransform(position, scale, transform.Rotation);

				Renderer::DrawQuad(transformMatrix, color);
			}
		}

		// Draw circle colliders
		auto ccView = m_ActiveScene->m_Registry.view<TransformComponent, CircleColliderComponent>();
		for (auto entity : ccView)
		{
			auto [transform, cc] = ccView.get<TransformComponent, CircleColliderComponent>(entity);

			// Check if current entity is selected and draw collider rect
			bool drawSelected = m_ShowSelectionCollider && m_SelectedEntity.m_Handle == entity;

			if (m_ShowAllColliders || drawSelected)
			{
				float zPos = (m_ShowAllColliders && drawSelected) ? 0.205f : 0.2f;
				glm::vec4 color = (m_ShowAllColliders && drawSelected)
					? glm::vec4{ 0.9f, 0.3f, 0.3f, 0.5f } : glm::vec4{ 0.9f, 0.6f, 0.3f, 0.5f };
				glm::vec3 position = { transform.WorldPosition.x + cc.Offset.x, transform.WorldPosition.y + cc.Offset.y, zPos };
				glm::vec3 scale = { cc.Radius * transform.Scale.x, cc.Radius * transform.Scale.y, 1.0f };
				glm::mat4 transformMatrix = Math::GetTransform(position, scale, transform.Rotation);

				Renderer::DrawCircle(transformMatrix, color);
			}
		}

		// Draw selected entity outline
		if (m_SelectedEntity.IsValid() && m_ShowSelectionOutline)
		{
			auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();
			float padding = glm::sqrt(m_ActiveScene->GetPrimaryCamera().GetZoomLevel()) * 0.05f;
			glm::vec3 position = { transform.WorldPosition.x, transform.WorldPosition.y, 0.21f };
			glm::vec3 scale = { transform.Scale.x + padding, transform.Scale.y + padding, 1.0f };
			glm::mat4 transformMatrix = Math::GetTransform(position, scale, transform.Rotation);

			glm::vec4 color = m_ShowSelectionOutline && m_MoveSelectedEntity
				? glm::vec4{ 0.8f, 0.8f, 0.2f, 1.0f } : glm::vec4{ 1.0f };

			if (m_SelectedEntity.HasComponent<SpriteComponent>())
			{
				auto& sprite = m_SelectedEntity.GetComponent<SpriteComponent>();
				const glm::uvec2& pixelSize = sprite.Sprite.GetPixelSize();
				if (sprite.Sprite)
					scale.x *= (float)pixelSize.x / (float)pixelSize.y;
			}
			Renderer::SetLineWidth(glm::min(50.0f * padding, 1.0f));
			Renderer::DrawDashedRect(transformMatrix, color, m_Camera->m_Camera.GetZoomLevel());
		}

		Renderer::EndScene();
	}

	void SceneViewportPanel::HandleImGuiDragAndDrop()
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_PREFAB");
			if (payload && m_ActiveScene)
			{
				const wchar_t* path_wchar = (const wchar_t*)payload->Data;
				std::filesystem::path path(path_wchar);

				Entity entity = PrefabManager::SpawnPrefab(m_ActiveScene, path.string());
				auto& transform = entity.GetComponent<TransformComponent>();
				glm::vec2 cameraPos = m_ActiveScene->GetCursorWorldPosition();
				entity.SetWorldPosition({ cameraPos.x, cameraPos.y, transform.WorldPosition.z });
			}
			payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE");
			if (payload)
			{
				const wchar_t* path_wchar = (const wchar_t*)payload->Data;
				std::filesystem::path path(path_wchar);
				std::string sceneFilepath = path.replace_extension().replace_extension().string();
				SceneManager::Load(sceneFilepath);
				SceneManager::SetActiveScene(sceneFilepath);
			}

			ImGui::EndDragDropTarget();
		}
	}

}
#endif
