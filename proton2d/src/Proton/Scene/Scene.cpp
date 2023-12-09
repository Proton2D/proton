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
		: m_SceneName(name), m_PhysicsWorld(MakeUnique<PhysicsWorld>(this))
	{
	}

	Scene::~Scene()
	{
		DestroyAll();
	}

	void Scene::BeginPlay()
	{	
		if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Paused)
			return; 

		if (m_EnablePhysics)
			m_PhysicsWorld->BuildWorld();

		m_SceneState = SceneState::Play;
	}

	void Scene::Pause(bool pause)
	{
		if (m_SceneState == SceneState::Stop)
			return;

		m_SceneState = pause ? SceneState::Paused : SceneState::Play;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID id, const std::string& name)
	{
		Entity entity = Entity{ m_Registry.create(), this };
		entity.AddComponent<IDComponent>().ID = id;
		entity.AddComponent<TagComponent>().Tag = name;
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<RelationshipComponent>();
		m_EntityMap[id] = entity;
		m_Root.push_back(entity);
		return entity;
	}

	void Scene::DestroyEntity(Entity entity, bool popHierarchy)
	{
		if (!entity.IsValid()) return;

		if (entity.HasComponent<ScriptComponent>())
			entity.TerminateScripts();

		if (entity.HasComponent<RigidbodyComponent>())
		{
			if (m_EnablePhysics && m_PhysicsWorld->IsInitialized())
				m_PhysicsWorld->DestroyRuntimeBody(entity.GetUUID());	
		}

		if (m_PrimaryCameraEntity == entity)
		{
			m_PrimaryCameraEntity = entt::null;
			m_PrimaryCamera = nullptr;
		}

		auto& rc = entity.GetComponent<RelationshipComponent>();

		// Destroy child entities
		Entity current{ rc.First, this };
		while (current)
		{
			entt::entity next = current.GetComponent<RelationshipComponent>().Next;
			DestroyEntity(current, false);
			current = Entity{ next, this };
		}

		// Update parent hierarchy only for entity which is being deleted
		if (popHierarchy && rc.Parent != entt::null)
		{
			Entity parent{ rc.Parent, this };
			Entity prev{ rc.Prev, this };
			Entity next{ rc.Next, this };

			auto& prc = parent.GetComponent<RelationshipComponent>();
			prc.ChildrenCount--;
			if (prc.First == entity.m_Handle)
				prc.First = rc.Next;

			if (prev)
				prev.GetComponent<RelationshipComponent>().Next = rc.Next;
		
			if (next)
				next.GetComponent<RelationshipComponent>().Prev = rc.Prev;
		}

		if (rc.Parent == entt::null) 
		{
			auto it = std::find(m_Root.begin(), m_Root.end(), entity);
			if (it != m_Root.end())
				m_Root.erase(it);
		}

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

	void Scene::SetEntityLocalPosition(Entity entity, const glm::vec3& position)
	{
		auto [transform, rc] = m_Registry.get<TransformComponent, RelationshipComponent>(entity.m_Handle);
		transform.WorldPosition = position;
		transform.LocalPosition = position;
		Entity current{ rc.Parent, this };
		while (current)
		{
			auto& crc = current.GetComponent<RelationshipComponent>();
			transform.WorldPosition += current.GetTransform().LocalPosition;
			current = Entity{ crc.Parent, this };
		}
	}

	void Scene::SetEntityWorldPosition(Entity entity, const glm::vec3& position)
	{
		auto [transform, rc] = m_Registry.get<TransformComponent, RelationshipComponent>(entity.m_Handle);
		transform.WorldPosition = position;
		transform.LocalPosition = position;
		Entity current{ rc.Parent, this };
		while (current)
		{
			auto& crc = current.GetComponent<RelationshipComponent>();
			transform.LocalPosition -= current.GetTransform().LocalPosition;
			current = Entity{ crc.Parent, this };
		}
	}

	static void CalculateEntityWorldPositon(Scene* scene, Entity entity, const glm::vec3& parentPos, RelationshipComponent& rc, bool isPhysicsSimulated)
	{
		auto& transform = entity.GetTransform();
		if (isPhysicsSimulated && entity.HasComponent<RigidbodyComponent>())
			scene->SetEntityWorldPosition(entity, transform.WorldPosition);
		else
			transform.WorldPosition = parentPos + transform.LocalPosition;
		if (rc.First != entt::null) 
		{
			Entity current{ rc.First, scene };
			while (current) 
			{
				auto& crc = current.GetComponent<RelationshipComponent>();
				CalculateEntityWorldPositon(scene, current, transform.WorldPosition, crc, isPhysicsSimulated);
				current = Entity{ crc.Next, scene};
			}
		}
	}

	void Scene::CalculateWorldPositions(bool isPhysicsSimulated)
	{
		for (auto& entity : m_Root)
		{
			auto& transform = entity.GetTransform();
			if (isPhysicsSimulated)
				transform.LocalPosition = transform.WorldPosition;
			else
				transform.WorldPosition = transform.LocalPosition;

			auto& rrc = entity.GetComponent<RelationshipComponent>();
			Entity current{ rrc.First, this };
			while (current)
			{
				auto& rc = current.GetComponent<RelationshipComponent>();
				CalculateEntityWorldPositon(this, current, transform.WorldPosition, rc, isPhysicsSimulated);
				current = Entity{ rc.Next, this };
			}
		}
	}

	void Scene::OnUpdate(float ts)
	{
		PROFILE_FUNCTION();
		
		CachePrimaryCameraPosition();
		CacheCursorWorldPosition();

		// Calculate world position (no physics simulation)
		if (m_SceneState != SceneState::Play)
			CalculateWorldPositions(false);

		if (m_SceneState == SceneState::Play)
		{
			// Update physics
			if (m_EnablePhysics && m_PhysicsWorld->IsInitialized())
			{
				PROFILE_SCOPE("update_physics");
				m_PhysicsWorld->Update(ts);

				// Calculate World Position (physics simulation)
				CalculateWorldPositions(true);
			}
			// Update scripts
			{
				PROFILE_SCOPE("update_scripts");
				m_Registry.view<ScriptComponent>().each([=](auto e, auto& component)
				{
					Entity entity{ e, this };
					bool hasRigidbodyComponent = entity.HasComponent<RigidbodyComponent>();

					for (auto& [scriptClassName, scriptInstance] : component.Scripts)
					{
						if (!scriptInstance->m_Initialized)
						{
							// Initialize EntityScript instance
							scriptInstance->m_Initialized = true;

							if (hasRigidbodyComponent && m_PhysicsWorld->IsInitialized())
								scriptInstance->RetrieveRuntimeBody();

							if (!scriptInstance->OnCreate()) 
							{
								scriptInstance->m_Stopped = true;
								continue;
							}
						}
						if(!scriptInstance->m_Stopped)
							scriptInstance->OnUpdate(ts);
					}
				});
			}
			// Update animations
			auto view = m_Registry.view<SpriteAnimationComponent>();
			for (auto entity : view)
				view.get<SpriteAnimationComponent>(entity).SpriteAnimation.Update(ts);
		}

		// Render scene
		RenderScene(GetPrimaryCamera());
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

			glm::mat4 transformMatrix = Math::GetTransform(transform.WorldPosition, scale, transform.Rotation);

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

			glm::mat4 transformMatrix = Math::GetTransform(transform.WorldPosition, glm::vec2{1.0f}, transform.Rotation);
			
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
			camera.Camera.SetAspectRatio((float)width / float(height));
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
		if (m_SceneState == SceneState::Stop || EditorLayer::GetCamera().m_UseInRuntime)
		{
			m_PrimaryCameraPosition = EditorLayer::GetCamera().GetPosition();
			return;
		}
	#endif
		if (m_PrimaryCameraEntity == entt::null) 
		{
			m_PrimaryCameraPosition = glm::vec3{ 0.0f };
			return;
		}
		auto [transform, camera] = m_Registry.get<TransformComponent, CameraComponent>(m_PrimaryCameraEntity);
		m_PrimaryCameraPosition = glm::vec3 {
			transform.WorldPosition.x + camera.PositionOffset.x,
			transform.WorldPosition.y + camera.PositionOffset.y, 0
		};
	}

	void Scene::CacheCursorWorldPosition()
	{
	#ifdef PT_EDITOR
		uint32_t width = (uint32_t)EditorLayer::Get()->m_SceneViewportPanel.m_ViewportSize.x;
		uint32_t height = (uint32_t)EditorLayer::Get()->m_SceneViewportPanel.m_ViewportSize.y;
		const glm::vec2& mouse = EditorLayer::Get()->m_SceneViewportPanel.m_MousePos;
	#else
		Window& window = Application::Get().GetWindow();
		uint32_t width = window.GetWidth();
		uint32_t height = window.GetHeight();
		glm::vec2 mouse = Input::GetMousePosition();
	#endif
		OrthoProjection ortho = GetPrimaryCamera().GetOrthoProjection();
		auto& camera = GetPrimaryCameraPosition();
		m_CursorWorldPosition[0] = mouse.x / (float)width * ortho.Right * 2.0f + camera.x + ortho.Left;
		m_CursorWorldPosition[1] = mouse.y / (float)height * ortho.Bottom * 2.0f + camera.y + ortho.Top;
	}

	Camera& Scene::GetPrimaryCamera()
	{
	#ifdef PT_EDITOR
		if (m_SceneState == SceneState::Stop || EditorLayer::GetCamera().m_UseInRuntime)
			return EditorLayer::GetCamera().GetBaseCamera();
	#endif
		return m_PrimaryCamera ? *m_PrimaryCamera : m_DefaultCamera;
	}

	void Scene::SetPrimaryCameraEntity(Entity entity)
	{
		if (!entity || !entity.HasComponent<CameraComponent>())
		{
			PT_CORE_ERROR_FUNCSIG("Entity does not have CameraComponent!");
			return;
		}

		auto& camera = entity.GetComponent<CameraComponent>();
		m_PrimaryCamera = &camera.Camera;
		m_PrimaryCameraEntity = entity.m_Handle;
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
			glm::vec2 rotationCenter = { transform.WorldPosition.x, transform.WorldPosition.y };
			point -= rotationCenter;
			point = {
				point.x * cosinus - point.y * sinus + rotationCenter.x,
				point.x * sinus + point.y * cosinus + rotationCenter.y
			};
		}

		// Check if point is inside entity bounding box
		const glm::vec3& position = transform.WorldPosition;
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
				glm::vec2 rotationCenter = { transform.WorldPosition.x, transform.WorldPosition.y };
				point -= rotationCenter;
				point = {
					point.x * cosinus - point.y * sinus + rotationCenter.x,
					point.x * sinus + point.y * cosinus + rotationCenter.y
				};
			}

			// Check if point is inside entity bounding box
			const glm::vec3& position = transform.WorldPosition;
			const glm::vec2& scale = transform.Scale;
			if (point.x >= position.x - scale.x / 2.0f && point.x <= position.x + scale.x / 2.0f
				&& point.y >= position.y - scale.y / 2.0f && point.y <= position.y + scale.y / 2.0f)
			{
				entities.emplace_back(Entity{ entity, this });
			}
		}
		return entities;
	}

	b2Body* Scene::GetRuntimeBody(UUID id)
	{
		PT_CORE_ASSERT(m_EnablePhysics && m_PhysicsWorld->IsInitialized(), "Physics world is not initialized");
		return m_PhysicsWorld->GetRuntimeBody(id);
	}

	const glm::vec3& Scene::GetPrimaryCameraPosition()
	{
		return m_PrimaryCameraPosition;
	}

	void Scene::SetScreenClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
	}

	uint32_t Scene::GetEntitiesCount() const
	{
		return (int32_t)m_Registry.alive();
	}

	uint32_t Scene::GetScriptedEntitiesCount() const
	{
		return (int32_t)m_Registry.view<ScriptComponent>().size();
	}

	const std::string& Scene::GetFilepath() const
	{
		return m_SceneFilepath;
	}

	SceneState Scene::GetSceneState() const
	{
		return m_SceneState;
	}

	bool Scene::IsPhysicsEnabled() const
	{
		return m_EnablePhysics;
	}

	bool Scene::IsPhysicsWorldInitialized() const
	{
		return m_PhysicsWorld->IsInitialized();
	}
}
