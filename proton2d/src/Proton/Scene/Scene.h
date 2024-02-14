#pragma once

#include "Proton/Graphics/Camera.h"
#include "Proton/Events/Event.h"
#include "Proton/Core/UUID.h"

#include <entt/entt.hpp>

class b2Body;

namespace proton {

	constexpr glm::vec4 DEFAULT_SCENE_SCREEN_CLEAR_COLOR = { 0.24f, 0.37f, 0.67f, 1.0f };

	// Forward declaration
	class Entity;
	class PhysicsWorld;

	enum class SceneState
	{
		Stop, Play, Paused
	};

	//
	// Scene Class
	// Wrapper for the Entity Registry (entt:registry) from the EnTT Entity Component System (ECS) library.
	// Provides additional functionalities to manage entities on the scene.
	//
	class Scene
	{
	public:
		Scene(const std::string& name = "Unnamed scene");
		virtual ~Scene();

		// Initialize physics world, begin simulation. Set scene state to SceneState::Play.
		// In Distribution build, this function is automaticlly being called
		// by SceneManager::SetActiveScene function.
		void BeginPlay();

		// Pause the simulation. Set scene state to SceneState::Pause
		void Pause(bool pause = true);
		
		// Create entitiy with random unique identifier (UUID)
		Entity CreateEntity(const std::string& name = "Entity");

		// Create entitiy with given identifier (UUID)
		Entity CreateEntityWithUUID(UUID id, const std::string& name = "Entity");

		// Destroy given entity
		void DestroyEntity(Entity entity, bool popHierachy = true);

		// Destroy all entities on the scene
		void DestroyAll();

		// Find entity by its unique id (UUID)
		Entity FindByID(UUID id);

		// Find entity by tag (name) from TagComponent
		Entity FindByTag(const std::string& tag);

		void SetEntityLocalPosition(Entity entity, const glm::vec3& position);
		void SetEntityWorldPosition(Entity entity, const glm::vec3& position);

		// Find all entities that have given tag
		std::vector<Entity> FindAllByTag(const std::string& tag);

		// Get entities with given set of components
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		// TODO: Add DuplicateEntity function

		// Camera related methods
		void SetPrimaryCameraEntity(Entity entity);
		Entity GetPrimaryCameraEntity();
		Camera& GetPrimaryCamera();
		const glm::vec3& GetPrimaryCameraPosition();

		// Cursor related methods
		const glm::vec2& GetCursorWorldPosition();
		bool IsCursorHoveringEntity(Entity entity);
		std::vector<Entity> GetEntitiesOnCursorLocation();

		// Get by UUID the Box2D body from physics world during game runtime. 
		// Entity must have RigidbodyComponent.
		b2Body* GetRuntimeBody(UUID id);

		// Set renderer screen clear color
		void SetScreenClearColor(const glm::vec4& color);

		uint32_t GetEntitiesCount() const;

		uint32_t GetScriptedEntitiesCount() const;

		// Get scene filepath (relative to "content/scenes" directory)
		const std::string& GetFilepath() const;

		// SceneState::Play, SceneState::Play, SceneState::Stop
		SceneState GetSceneState() const;

		bool IsPhysicsEnabled() const;

		bool IsPhysicsWorldInitialized() const;

	private:
		void OnUpdate(float ts);
		void RenderScene(const Camera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);
		void DestroyChildEntities(Entity entity);

		void CachePrimaryCameraPosition();
		void CacheCursorWorldPosition();

		void CalculateWorldPositions(bool isPhysicsSimulated);

	private:
		// TODO: Add Scene UUID
		SceneState m_SceneState = SceneState::Stop;

		// General
		std::string m_SceneName;
		std::string m_SceneFilepath = "<Unsaved scene>";
		glm::vec4 m_ClearColor = DEFAULT_SCENE_SCREEN_CLEAR_COLOR;

		// ECS
		entt::registry m_Registry;
		std::unordered_map<UUID, Entity> m_EntityMap;
		std::vector<Entity> m_Root;

		// Camera
		entt::entity m_PrimaryCameraEntity = entt::null;
		Camera* m_PrimaryCamera = nullptr;
		Camera m_DefaultCamera; // pass this to Renderer if m_PrimaryCamera is nullptr

		// Physics
		bool m_EnablePhysics = true;
		Unique<PhysicsWorld> m_PhysicsWorld;

		// Cache
		glm::vec3 m_PrimaryCameraPosition = { 0.0f, 0.0f, 0.0f };
		glm::vec2 m_CursorWorldPosition = { 0.0f, 0.0f };

		friend class Application;
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneManager;
		friend class PhysicsWorld;
		
		friend class EditorLayer;
		friend class EditorCamera;
		friend class SettingsPanel;
		friend class InspectorPanel;
		friend class SceneHierarchyPanel;
		friend class ToolbarPanel;
		friend class EditorMenuBar;
		friend class SceneViewportPanel;
	};

}
