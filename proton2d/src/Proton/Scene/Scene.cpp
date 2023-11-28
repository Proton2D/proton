#include "ptpch.h"
#include "Proton/Scene/Scene.h"
#include "Proton/Scene/Entity.h"
#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Scripting/EntityScript.h"
#include "Proton/Core/Application.h"
#include "Proton/Core/Input.h"
#include "Proton/Assets/SceneSerializer.h"
#include "Proton/Utils/Utils.h"
#include "Proton/Physics/PhysicsWorld.h"

#ifdef PT_EDITOR
#include "Proton/Editor/EditorLayer.h"
#include <imgui.h>
#endif

namespace proton {

	Scene::Scene(const std::string& name)
		: m_SceneName(name),
		m_DefaultCamera(MakeShared<Camera>()),
		m_PhysicsWorld(new PhysicsWorld(this))
	{
	}

	Scene::~Scene()
	{
		DestroyAll();
		delete m_PhysicsWorld;
	}

	void Scene::BeginPlay()
	{	
		if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Paused)
			return;

	#ifdef PT_EDITOR
		// TODO: Refactor: Change to Scene::DuplicateScene
		// Temporary solution
		SceneSerializer serializer(this);
		std::string filepath = m_SceneFilepath == "<Unsaved scene>" ? "unsaved_scene" : m_SceneFilepath;
		std::replace(filepath.begin(), filepath.end(), '\\', '_');
		serializer.Serialize("editor/cache/" + filepath + ".scene.json");
	#endif

		if (m_EnablePhysics)
			m_PhysicsWorld->BuildWorld();

		m_SceneState = SceneState::Play;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithID(UUID(), name);
	}

	Entity Scene::CreateEntityWithID(UUID id, const std::string& name)
	{
		Entity entity = Entity{ m_Registry.create(), this };
		entity.AddComponent<IDComponent>().ID = id;
		entity.AddComponent<TagComponent>().Tag = name;
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<RelationshipComponent>();
		m_EntityMap[id] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity, bool popHierarchy)
	{
		if (!entity.IsValid())
			return;

		if (entity.HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity.GetComponent<ScriptComponent>();
			for (auto& kv : scriptComponent.Scripts)
				kv.second->OnDestroy();

			entity.TerminateScripts();
		}

		if (entity.HasComponent<RigidbodyComponent>())
		{
			if (m_EnablePhysics && m_PhysicsWorld->IsIntialized())
				m_PhysicsWorld->DestroyRuntimeBody(entity.GetUUID());	
		}

		if (m_PrimaryCameraEntity == entity)
		{
			m_PrimaryCameraEntity = entt::null;
			m_PrimaryCamera = nullptr;
		}

		DestroyChildEntities(entity);
		if (popHierarchy)
			entity.PopHierarchy();
			
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity.m_Handle);
	}

	void Scene::DestroyAll()
	{
		m_Registry.each([&](entt::entity e) 
		{
			Entity entity{ e, this };
			DestroyEntity(entity);
		});
	}

	void Scene::Pause(bool pause)
	{
		m_SceneState = pause ? SceneState::Paused : SceneState::Play;
	}

	void Scene::Stop()
	{
		if (m_SceneState == SceneState::Stop) {
			PT_CORE_WARN("[Scene::Stop] Scene simulation is already stopped.");
			return;
		}
		
		m_PhysicsWorld->DestroyWorld();

		m_Registry.view<ScriptComponent>().each([=](auto entity, auto& scriptComponent)
		{
			for (auto& [scriptName, scriptInstance] : scriptComponent.Scripts)
			{
				if (scriptInstance->m_Initialized)
					Entity{ entity, this }.RemoveScript(scriptName);
			}
		});

		m_SceneState = SceneState::Stop;
	}

	void Scene::SetScreenClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
	}

	void Scene::OnUpdate(float ts)
	{
		PROFILE_FUNCTION();
		
		CachePrimaryCameraPosition();
		CacheCursorWorldPosition();

		if (m_SceneState == SceneState::Play)
		{
			// Update physics
			if (m_EnablePhysics && m_PhysicsWorld->IsIntialized())
			{
				PROFILE_SCOPE("update_physics");
				m_PhysicsWorld->Update(ts);
			}
			// Update scripts
			{
				PROFILE_SCOPE("update_scripts");
				m_Registry.view<ScriptComponent>().each([=](auto entity, auto& component)
				{
					for (auto& [scriptName, scriptInstance] : component.Scripts)
					{
						if (!scriptInstance->m_Initialized)
						{
							// Create script instance
							scriptInstance->m_Entity = Entity{ entity, this };
							scriptInstance->OnCreate();
							scriptInstance->m_Initialized = true;
						}
						scriptInstance->OnUpdate(ts);
					}
				});
			}
			// Update animations
			auto view = m_Registry.view<SpriteAnimationComponent>();
			for (auto entity : view)
				view.get<SpriteAnimationComponent>(entity).SpriteAnimation->Update(ts);
		}

		// Render scene
		RenderScene(*GetPrimaryCamera());
	}

	Entity Scene::FindByID(UUID id)
	{
		if (m_EntityMap.find(id) != m_EntityMap.end())
			return m_EntityMap.at(id);
		return Entity();
	}

	Entity Scene::FindByTag(const std::string& tag)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			if (tag == view.get<TagComponent>(entity).Tag)
				return Entity(entity, this);
		}
		return Entity();
	}

	std::vector<Entity> Scene::FindAllByTag(const std::string& tag)
	{
		auto view = m_Registry.view<TagComponent>();
		std::vector<Entity> entities;
		entities.reserve(view.size());
		for (auto entity : view) 
		{
			if (tag == view.get<TagComponent>(entity).Tag)
				entities.emplace_back(Entity(entity, this));
		}
		return entities;
	}

	void Scene::RenderScene(const Camera& camera)
	{
		PROFILE_FUNCTION();
		Renderer::BeginScene(camera, GetPrimaryCameraPosition());

		// Render entities with SpriteComponent
		auto renderableSprite = m_Registry.view<SpriteComponent, TransformComponent>();
		for (auto e : renderableSprite)
		{
			PROFILE_SCOPE("entity_render_sprite");
			auto [transform, sprite] = renderableSprite.get<TransformComponent, SpriteComponent>(e);
			
			// Sprite mirror flip
			glm::vec3 scale = {
				transform.Scale.x * (sprite.Sprite.m_MirrorFlipX ? -1.0f : 1.0f),
				transform.Scale.y * (sprite.Sprite.m_MirrorFlipY ? -1.0f : 1.0f), 1.0f
			};

			glm::mat4 transformMatrix = Math::GetTransform(transform.Position, scale, transform.Rotation);

			if (sprite.Sprite)
				Renderer::DrawQuad(transformMatrix, sprite.Sprite, sprite.Color, sprite.TilingFactor);
			else
				Renderer::DrawQuad(transformMatrix, sprite.Color, sprite.TilingFactor);
		}

		// Render entities with ResizableSpriteComponent
		// TODO: Think about using TilingFactor in shader program for center tiles
		auto renderableResizableSprite = m_Registry.view<TransformComponent, ResizableSpriteComponent>();
		for (auto e : renderableResizableSprite)
		{
			PROFILE_SCOPE("entity_render_resizable_sprite");
			auto [transform, rsc] = renderableResizableSprite.get<TransformComponent, ResizableSpriteComponent>(e);
			auto& sprite = rsc.ResizableSprite;
			auto& spritesheet = sprite.m_Spritesheet;

			if (!spritesheet)
				continue;

			glm::mat4 transformMatrix = Math::GetTransform(transform.Position, glm::vec2{1.0f}, transform.Rotation);
			
			for (const auto& column : sprite.m_Tilemap)
				for (const auto& tile : column)
				{
					Renderer::DrawQuad(transformMatrix * tile.LocalTransform,
						spritesheet->GetTexture(), tile.Coords, rsc.Color);
				}
		}

		Renderer::EndScene();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			camera.Camera->SetAspectRatio((float)width / float(height));
		}
	}

	void Scene::DestroyChildEntities(Entity entity)
	{
		auto& rc = entity.GetComponent<RelationshipComponent>();
		Entity current{ rc.First, this };
		while (current)
		{
			entt::entity next = current.GetComponent<RelationshipComponent>().Next;
			DestroyEntity(current, false);
			current = Entity{ next, this };
		}
		rc.ChildrenCount = 0;
		rc.First = entt::null;
	}

	void Scene::CachePrimaryCameraPosition()
	{
	#ifdef PT_EDITOR
		if (m_SceneState != SceneState::Stop && !EditorLayer::Get()->m_UseEditorCameraInRuntime)
		{
	#endif
			if (m_PrimaryCameraEntity == entt::null) 
			{
				m_PrimaryCameraPosition = glm::vec3{ 0.0f };
				return;
			}
			auto [transform, camera] = m_Registry.get<TransformComponent, CameraComponent>(m_PrimaryCameraEntity);
			m_PrimaryCameraPosition = glm::vec3 {
				transform.Position.x + camera.PositionOffset.x,
				transform.Position.y + camera.PositionOffset.y, 0
			};
	#ifdef PT_EDITOR
			return;
		}
		m_PrimaryCameraPosition = EditorLayer::GetCamera().GetPosition();
	#endif
	}

	void Scene::CacheCursorWorldPosition()
	{
#ifdef PT_EDITOR
		uint32_t width = (uint32_t)EditorLayer::s_Instance->m_ViewportSize.x;
		uint32_t height = (uint32_t)EditorLayer::s_Instance->m_ViewportSize.y;
#else
		Window& window = Application::Get().GetWindow();
		uint32_t width = window.GetWidth(), height = window.GetHeight();
#endif
		OrthoProjection ortho = GetPrimaryCamera()->GetOrthoProjection();
		auto& camera = GetPrimaryCameraPosition();
#ifdef PT_EDITOR
		const glm::vec2& mouse = EditorLayer::s_Instance->m_MousePos;
#else
		glm::vec2 mouse = Input::GetMousePosition();
#endif
		m_CursorWorldPosition[0] = mouse.x / (float)width * ortho.Right * 2.0f + camera.x + ortho.Left;
		m_CursorWorldPosition[1] = mouse.y / (float)height * ortho.Bottom * 2.0f + camera.y + ortho.Top;
	}

	b2Body* Scene::GetRuntimeBody(UUID id)
	{
		PT_ASSERT(m_EnablePhysics && m_PhysicsWorld->IsIntialized(), "Physics world is not initialized");
		return m_PhysicsWorld->GetRuntimeBody(id);
	}

	// TODO: Remove
	b2Body* Scene::CreateRuntimeBody(Entity entity)
	{
		if (!m_EnablePhysics || !m_PhysicsWorld->IsIntialized()) return nullptr;
		return m_PhysicsWorld->CreateRuntimeBody(entity);
	}

	void Scene::SetPrimaryCameraEntity(Entity entity)
	{
		if (!entity || !entity.HasComponent<CameraComponent>())
		{
			PT_CORE_ERROR("[Scene::SetPrimaryCameraEntity] Entity does not have CameraComponent!");
			return;
		}

		auto& camera = entity.GetComponent<CameraComponent>();
		m_PrimaryCamera = camera.Camera;
		m_PrimaryCameraEntity = entity.m_Handle;
	}

	Shared<Camera> Scene::GetPrimaryCamera()
	{
	#ifdef PT_EDITOR
		if (m_SceneState != SceneState::Stop) {
	#endif
		// TODO: Refactor this and Renderer::RenderScene
		return m_PrimaryCamera ? m_PrimaryCamera : m_DefaultCamera;
	#ifdef PT_EDITOR
		}
		return EditorLayer::GetCamera().GetBaseCamera();
	#endif
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		return Entity{ m_PrimaryCameraEntity, this };
	}

	const glm::vec2& Scene::GetCursorWorldPosition()
	{
		return m_CursorWorldPosition;
	}

	bool Scene::IsCursorHoveringEntity(Entity entity)
	{
		auto& transform = entity.GetComponent<TransformComponent>();

		// Apply entity rotation to point (mouse position)
		// so we can easliy check after if point is inside rectangle
		float sinus = sin(glm::radians(-transform.Rotation));
		float cosinus = cos(glm::radians(-transform.Rotation));
		glm::vec2 point = GetCursorWorldPosition();
		if (transform.Rotation)
		{
			glm::vec2 rotationCenter = { transform.Position.x, transform.Position.y };
			point -= rotationCenter;
			point = {
				point.x * cosinus - point.y * sinus + rotationCenter.x,
				point.x * sinus + point.y * cosinus + rotationCenter.y
			};
		}

		// Check if point is inside entity bounding box
		const glm::vec3& position = transform.Position;
		const glm::vec2& scale = transform.Scale;
		return point.x >= position.x - scale.x / 2.0f && point.x <= position.x + scale.x / 2.0f
			&& point.y >= position.y - scale.y / 2.0f && point.y <= position.y + scale.y / 2.0f;
	}

	std::vector<Entity> Scene::GetEntitiesOnCursorLocation()
	{
		// TODO: Optimize / refactor
		const glm::vec2& mousePos = GetCursorWorldPosition();
		auto view = m_Registry.view<TransformComponent>();
		std::vector<Entity> entities;

		for (auto entity : view)
		{
			auto& transform = view.get<TransformComponent>(entity);

			// Apply entity rotation to point (mouse position)
			// so we can easliy check after if point is inside rectangle
			float sinus = sin(glm::radians(-transform.Rotation));
			float cosinus = cos(glm::radians(-transform.Rotation));
			glm::vec2 point = mousePos;
			if (transform.Rotation)
			{
				glm::vec2 rotationCenter = { transform.Position.x, transform.Position.y };
				point -= rotationCenter;
				point = {
					point.x * cosinus - point.y * sinus + rotationCenter.x,
					point.x * sinus + point.y * cosinus + rotationCenter.y
				};
			}

			// Check if point is inside entity bounding box
			const glm::vec3& position = transform.Position;
			const glm::vec2& scale = transform.Scale;
			if (point.x >= position.x - scale.x / 2.0f && point.x <= position.x + scale.x / 2.0f
				&& point.y >= position.y - scale.y / 2.0f && point.y <= position.y + scale.y / 2.0f)
			{
				entities.emplace_back(Entity{ entity, this });
			}
		}
		return entities;
	}

	const glm::vec3& Scene::GetPrimaryCameraPosition()
	{
		return m_PrimaryCameraPosition;
	}

	uint32_t Scene::GetEntitiesCount() const
	{
		return (int32_t)m_Registry.alive();
	}

	uint32_t Scene::GetScriptedEntitiesCount() const
	{
		return (int32_t)m_Registry.view<ScriptComponent>().size();
	}
}
