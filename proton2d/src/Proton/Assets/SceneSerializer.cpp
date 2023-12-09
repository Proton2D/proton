#include "ptpch.h"
#include "Proton/Assets/SceneSerializer.h"
#include "Proton/Scene/EntityComponent.h"
#include "Proton/Scene/Scene.h"
#include "Proton/Assets/AssetManager.h"
#include "Proton/Scripting/EntityScript.h"
#include "Proton/Scripting/ScriptFactory.h"
#include "Proton/Physics/PhysicsWorld.h"
#include "Proton/Utils/Utils.h"

#include <fstream>

#define PROTON_SERIALIZER_INDENT_JSON 0

namespace proton {

	static inline double round(float f)
	{
		return std::round((double)f * 100000) / 100000;
	}

	SceneSerializer::SceneSerializer(Scene* scene)
		: m_Scene(scene)
	{
	}

	// *****************************************
	//         Serialize Scene Function
	// *****************************************

	bool SceneSerializer::Serialize(const std::string& filepath)
	{
		PT_CORE_ASSERT(m_Scene, "Scene context not set!");
		const auto& c = m_Scene->m_ClearColor;
		json jsonObj = {
			{ "SceneName",          m_Scene->m_SceneName },
			{ "EnablePhysics",      m_Scene->m_EnablePhysics },
			{ "GravityForce",       m_Scene->m_PhysicsWorld->m_Gravity },
			{ "VelocityIterations", m_Scene->m_PhysicsWorld->m_PhysicsVelocityIterations },
			{ "PositionIterations", m_Scene->m_PhysicsWorld->m_PhysicsPositionIterations },
			{ "ScreenClearColor", { c.r, c.g, c.b, c.a } }
		};

		Entity primaryCameraEntity = m_Scene->GetPrimaryCameraEntity();
		if (primaryCameraEntity.IsValid())
		{
			uint64_t id = m_Scene->GetPrimaryCameraEntity().GetUUID();
			jsonObj["PrimaryCameraEntity"] = id;
		}

		m_Scene->m_Registry.each([&](auto id)
			{
				Entity entity{ id, m_Scene };
				auto& relationship = entity.GetComponent<RelationshipComponent>();
				if (relationship.Parent == entt::null)
					jsonObj["Entities"].push_back(SerializeEntity(entity));
			});

		std::ofstream out(filepath);
	#if PROTON_SERIALIZER_INDENT_JSON
		out << jsonObj.dump(4);
	#else
		out << jsonObj;
	#endif
		out.close();
		return true;
	}

	// *****************************************
	//       Deserialize Scene Function
	// *****************************************

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::string jsonData = Utils::ReadFile(filepath);
		if (jsonData.size())
		{
			json jsonObj = json::parse(jsonData);
			m_Scene->m_SceneName = jsonObj["SceneName"];
			m_Scene->m_EnablePhysics = jsonObj["EnablePhysics"];
			m_Scene->m_PhysicsWorld->m_Gravity = jsonObj["GravityForce"];
			m_Scene->m_PhysicsWorld->m_PhysicsVelocityIterations = jsonObj["VelocityIterations"];
			m_Scene->m_PhysicsWorld->m_PhysicsPositionIterations = jsonObj["PositionIterations"];
			json& c = jsonObj["ScreenClearColor"];
			m_Scene->m_ClearColor = { c[0], c[1], c[2], c[3] };

			json& entities = jsonObj["Entities"];
			for (auto it = entities.rbegin(); it != entities.rend(); it++)
				DeserializeEntity(*it);

			if (jsonObj.contains("PrimaryCameraEntity"))
			{
				UUID id{ jsonObj["PrimaryCameraEntity"] };
				m_Scene->SetPrimaryCameraEntity(m_Scene->FindByID(id));
			}
			m_Scene->CalculateWorldPositions(false);
			return true;
		}
		return false;
	}

	// *****************************************
	//       Serialize Entity Function
	// *****************************************

	json SceneSerializer::SerializeEntity(Entity entity, bool serializeUUID)
	{
		json jsonObj;

		// Serialize IDComponent
		if (serializeUUID)
		{
			auto& uuid = entity.GetUUID();
			jsonObj["UUID"] = (uint64_t)uuid;
		}

		// Serialize TagComponent
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		jsonObj["Tag"] = tag;

		// Serialize TransformComponent
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& position = transform.LocalPosition;
		jsonObj["Transform"] = 
		{
			{ "Position", { round(position.x), round(position.y), round(position.z) } },
			{ "Rotation", round(transform.Rotation) },
			{ "Scale", { round(transform.Scale.x), round(transform.Scale.y) } }
		};

		// Serialize SpriteComponent
		if (entity.HasComponent<SpriteComponent>())
		{
			auto& spriteComponent = entity.GetComponent<SpriteComponent>();
			auto& sprite = spriteComponent.Sprite;
			auto& color = spriteComponent.Color;
			if (sprite)
			{
				jsonObj["Sprite"] = {
					{ "Texture",    sprite.GetTexture()->GetPath() },
					{ "FilterMode", sprite.GetTexture()->GetFilterMode() },
					{ "Flip", { sprite.m_MirrorFlipX, sprite.m_MirrorFlipY } }
				};

				if (sprite.m_Spritesheet)
				{
					jsonObj["Sprite"]["TilePos"] = { sprite.m_TilePos.x, sprite.m_TilePos.y }; 
					jsonObj["Sprite"]["TileSize"] = { sprite.m_TileSize.x, sprite.m_TileSize.y };
				}
			}
			jsonObj["Sprite"]["Color"] = { round(color.r), round(color.g), round(color.b), round(color.a) };
		}
		
		// Serialize ResizableSpriteComponent
		if (entity.HasComponent<ResizableSpriteComponent>())
		{
			auto& component = entity.GetComponent<ResizableSpriteComponent>();
			auto& sprite = component.ResizableSprite;
			auto& spritesheet = sprite.GetSpritesheet();
			const auto& col = component.Color;

			jsonObj["ResizableSprite"] = {
				{ "Width",     sprite.m_Width },
				{ "Height",    sprite.m_Width },
				{ "TileScale", sprite.m_TileScale },
				{ "Edges",     sprite.GetEdges() },
				{ "Color", { col.r, col.g, col.b, col.a } }
			};

			if (spritesheet)
				jsonObj["ResizableSprite"]["Spritesheet"] = spritesheet->GetTexture()->GetPath();
		}

		// Serialize RigidbodyComponent
		if (entity.HasComponent<RigidbodyComponent>())
		{
			auto& rb = entity.GetComponent<RigidbodyComponent>();
			jsonObj["Rigidbody"] = {
				{ "Type", rb.Type },
				{ "FixedRotation", rb.FixedRotation }
			};
		}

		// Serialize BoxColliderComponent
		if (entity.HasComponent<BoxColliderComponent>())
		{
			auto& collider = entity.GetComponent<BoxColliderComponent>();
			jsonObj["BoxCollider"] = {
				{ "Size",               { collider.Size.x,   collider.Size.y } },
				{ "Offset",             { collider.Offset.x, collider.Offset.y } },
				{ "Friction",             collider.Material.Friction },
				{ "Restitution",          collider.Material.Restitution },
				{ "RestitutionThreshold", collider.Material.RestitutionThreshold },
				{ "Density",              collider.Material.Density },
				{ "IsSensor",             collider.IsSensor }
			};
		}

		// Serialize CameraComponent
		if (entity.HasComponent<CameraComponent>())
		{
			auto& camera = entity.GetComponent<CameraComponent>();
			jsonObj["Camera"] = {
				{ "ZoomLevel", camera.Camera.GetZoomLevel() },
				{ "PositionOffset", { camera.PositionOffset.x, camera.PositionOffset.y } },
			};
		}

		// Serialize ScriptComponent
		if (entity.HasComponent<ScriptComponent>())
		{
			auto& script = entity.GetComponent<ScriptComponent>();
			for (auto& [scriptClassName, scriptInstance] : script.Scripts)
			{
				json scriptObj;
				scriptObj["ClassName"] = scriptClassName;
				for (auto& [fieldName, fieldData] : scriptInstance->m_ScriptFields)
				{
					json fieldObj;
					fieldObj["FieldName"] = fieldName;

					switch (fieldData.Type)
					{
					case ScriptFieldType::Float:
						fieldObj["Value"] = *(float*)fieldData.InstanceFieldValue;
						break;
					case ScriptFieldType::Float2:
					{
						const glm::vec2& data = *(glm::vec2*)fieldData.InstanceFieldValue;
						fieldObj["Value"] = { data.x, data.y };
						break;
					}
					case ScriptFieldType::Float3:
					{
						const glm::vec3& data = *(glm::vec3*)fieldData.InstanceFieldValue;
						fieldObj["Value"] = { data.x, data.y, data.z };
						break;
					}
					case ScriptFieldType::Float4:
					{
						const glm::vec4& data = *(glm::vec4*)fieldData.InstanceFieldValue;
						fieldObj["Value"] = { data.x, data.y, data.z, data.a };
						break;
					}

					case ScriptFieldType::Int:
						fieldObj["Value"] = *(int*)fieldData.InstanceFieldValue;
						break;
					case ScriptFieldType::Int2:
					{
						const glm::ivec2& data = *(glm::ivec2*)fieldData.InstanceFieldValue;
						fieldObj["Value"] = { data.x, data.y };
						break;
					}
					case ScriptFieldType::Int3:
					{
						const glm::ivec3& data = *(glm::ivec3*)fieldData.InstanceFieldValue;
						fieldObj["Value"] = { data.x, data.y, data.z };
						break;
					}
					case ScriptFieldType::Int4:
					{
						const glm::ivec4& data = *(glm::ivec4*)fieldData.InstanceFieldValue;
						fieldObj["Value"] = { data.x, data.y, data.z, data.a };
						break;
					}

					case ScriptFieldType::Bool:
						fieldObj["Value"] = *(bool*)fieldData.InstanceFieldValue;
						break;
					}
					scriptObj["Fields"].push_back(fieldObj);
				}
				jsonObj["Scripts"].push_back(scriptObj);
			}
		}

		// Serialize child entities
		auto& relationship = entity.GetComponent<RelationshipComponent>();
		if (relationship.ChildrenCount)
		{
			entt::entity current = relationship.First;
			while (current != entt::null)
			{
				Entity child{ current, entity.m_Scene };
				auto& rc = child.GetComponent<RelationshipComponent>();
				jsonObj["Entities"].push_back(SerializeEntity(child, serializeUUID));
				current = rc.Next;
			}
		}

		return jsonObj;
	}

	// *****************************************
	//       Deserialize Entity Function
	// *****************************************

	Entity SceneSerializer::DeserializeEntity(json jsonObj, bool deserializeUUID)
	{
		Entity entity = deserializeUUID ? 
			m_Scene->CreateEntityWithUUID((uint64_t)jsonObj["UUID"], jsonObj["Tag"]) :
			m_Scene->CreateEntity(jsonObj["Tag"]);

		// Deserialize TransformComponent
		auto& transform = entity.GetComponent<TransformComponent>();
		json& position = jsonObj["Transform"]["Position"];
		json& scale    = jsonObj["Transform"]["Scale"];
		json& rotation = jsonObj["Transform"]["Rotation"];
		transform.WorldPosition = { position[0], position[1], position[2] };
		transform.LocalPosition = { position[0], position[1], position[2] };
		transform.Scale    = { scale[0], scale[1] };
		transform.Rotation = rotation;

		// Deserialize SpriteComponent
		if (jsonObj.contains("Sprite"))
		{
			json& sprite = jsonObj["Sprite"];
			auto& spriteComponent = entity.AddComponent<SpriteComponent>();
			
			if (sprite.contains("Texture"))
			{
				const auto& texture = AssetManager::GetTexture(sprite["Texture"]);
				if (texture)
				{
					if (sprite.contains("TilePos"))
					{
						const auto& spritesheet = AssetManager::GetSpritesheet(sprite["Texture"]);
						if (spritesheet)
						{
							spriteComponent.Sprite.SetSpritesheet(spritesheet);
							spriteComponent.Sprite.SetTile(sprite["TilePos"][0], sprite["TilePos"][1]);
							spriteComponent.Sprite.SetTileSize(sprite["TileSize"][0], sprite["TileSize"][1]);
						}
						else
							PT_CORE_ERROR_FUNCSIG("Spritesheet {} does not exist!", sprite["Texture"]);
					}
					else
						spriteComponent.Sprite.SetTexture(texture);

					spriteComponent.Sprite.GetTexture()->m_FilterMode = sprite["FilterMode"];
					spriteComponent.Sprite.m_MirrorFlipX = sprite["Flip"][0];
					spriteComponent.Sprite.m_MirrorFlipX = sprite["Flip"][1];
				}
				else
					PT_CORE_ERROR_FUNCSIG("Texture '{}' does not exist!", sprite["Texture"]);
			}
			
			json& color = jsonObj["Sprite"]["Color"];
			spriteComponent.Color = { color[0], color[1], color[2], color[3] };
		}

		// Deserialize ResizableSpriteComponent
		if (jsonObj.contains("ResizableSprite"))
		{
			json& jsonData = jsonObj["ResizableSprite"];
			auto& component = entity.AddComponent<ResizableSpriteComponent>();
			auto& sprite = component.ResizableSprite;
			sprite.m_Edges = jsonData["Edges"];
			sprite.m_TileScale = jsonData["TileScale"];
			auto& c = jsonData["Color"];
			component.Color = { c[0], c[1], c[2], c[3] };

			if (jsonData.contains("Spritesheet"))
				sprite.SetSpritesheet(AssetManager::GetSpritesheet(jsonData["Spritesheet"]));

			sprite.Generate();
		}

		// Deserialize CameraComponent
		if (jsonObj.contains("Camera"))
		{
			auto& camera = entity.AddComponent<CameraComponent>();
			json& cameraJson = jsonObj["Camera"];
			camera.Camera.SetZoomLevel(cameraJson["ZoomLevel"]);
			camera.PositionOffset = { cameraJson["PositionOffset"][0], cameraJson["PositionOffset"][1] };
		}

		// Deserialize BoxColliderComponent
		if (jsonObj.contains("BoxCollider"))
		{
			auto& collider = entity.AddComponent<BoxColliderComponent>();
			json& boxCollider = jsonObj["BoxCollider"];
			json& size = boxCollider["Size"];
			json& offset = boxCollider["Offset"];

			collider.Size = { size[0], size[1] };
			collider.Offset = { offset[0], offset[1] };
			collider.Material.Friction = boxCollider["Friction"];
			collider.Material.Restitution = boxCollider["Restitution"];
			collider.Material.RestitutionThreshold = boxCollider["RestitutionThreshold"];
			collider.Material.Density = boxCollider["Density"];
			collider.IsSensor = boxCollider["IsSensor"];
		}

		// Deserialize RigidbodyComponent
		if (jsonObj.contains("Rigidbody"))
		{
			auto& rb = entity.AddComponent<RigidbodyComponent>();
			rb.Type = jsonObj["Rigidbody"]["Type"];
			rb.FixedRotation = jsonObj["Rigidbody"]["FixedRotation"];
			if (m_Scene->GetSceneState() != SceneState::Stop)
				m_Scene->m_PhysicsWorld->CreateRuntimeBody(entity);
		}

		// Deserialize scripts
		if (jsonObj.contains("Scripts"))
		{
			for (auto& scriptJson : jsonObj["Scripts"])
			{
				std::string scriptClassName = scriptJson["ClassName"];
				EntityScript* script = ScriptFactory::Get().AddScriptToEntity(entity, scriptClassName);

				if (script == nullptr)
					continue;

				if (scriptJson.contains("Fields"))
				{
					json& fields = scriptJson["Fields"];
					for (auto& field : fields)
					{
						std::string fieldName = field["FieldName"];
						if (script->m_ScriptFields.find(fieldName) != script->m_ScriptFields.end())
						{
							ScriptField& scriptField = script->m_ScriptFields[fieldName];
							json& value = field["Value"];

							switch (scriptField.Type)
							{
							case ScriptFieldType::Float:
								*(float*)scriptField.InstanceFieldValue = value;
								break;
							case ScriptFieldType::Float2:
								*(glm::vec2*)scriptField.InstanceFieldValue = { value[0], value[1] };
								break;											
							case ScriptFieldType::Float3:						
								*(glm::vec3*)scriptField.InstanceFieldValue = { value[0], value[1], value[2] };
								break;											
							case ScriptFieldType::Float4:						
								*(glm::vec4*)scriptField.InstanceFieldValue = { value[0], value[1], value[2], value[3] };
								break;

							case ScriptFieldType::Int:
								*(int*)scriptField.InstanceFieldValue = value;
								break;
							case ScriptFieldType::Int2:
								*(glm::ivec2*)scriptField.InstanceFieldValue = { value[0], value[1] };
								break;
							case ScriptFieldType::Int3:
								*(glm::ivec3*)scriptField.InstanceFieldValue = { value[0], value[1], value[2] };
								break;
							case ScriptFieldType::Int4:
								*(glm::ivec4*)scriptField.InstanceFieldValue = { value[0], value[1], value[2], value[3] };
								break;

							case ScriptFieldType::Bool:
								*(bool*)scriptField.InstanceFieldValue = value;
								break;
							}
						}
					}
				}
			}
		}

		// Deserialize child entities
		if (jsonObj.contains("Entities"))
		{
			json& entities = jsonObj["Entities"];
			for (auto it = entities.rbegin(); it != entities.rend(); it++)
				entity.AddChildEntity(DeserializeEntity(*it, deserializeUUID), false);
		}

		return entity;
	}

}
