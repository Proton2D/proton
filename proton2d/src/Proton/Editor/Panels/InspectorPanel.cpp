#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/InspectorPanel.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Utils/Utils.h"
#include "Proton/Assets/AssetManager.h"
#include "Proton/Scripting/ScriptFactory.h"
#include "Proton/Scene/Components.h"
#include "Proton/Scripting/EntityScript.h"
#include "Proton/Scene/PrefabManager.h"
#include "Proton/Physics/PhysicsWorld.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace proton {

	void InspectorPanel::OnImGuiRender()
	{
		ImGui::Begin("Inspector");

		if (!m_ActiveScene) {
			ImGui::End();
			return;
		}

		// Draw scene proporties if no entity is selected
		if (!m_SelectedEntity.IsValid())
		{
			DrawSceneProporties();
			ImGui::End();
			return;
		}

		// Display UUID of entity
		std::stringstream hexUUID;
		hexUUID << std::hex << (uint64_t)m_SelectedEntity.GetUUID();
		std::string uuid = "Entity " + hexUUID.str();
		ImGui::Columns(2, NULL, false);
		ImGui::SetColumnWidth(0, 190.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
		ImGui::Text(uuid.c_str());

		// Add component popup
		ImGui::NextColumn();
		if (ImGui::Button("       + Add        "))
			ImGui::OpenPopup("Add component");

		if (ImGui::BeginPopup("Add component"))
		{
			#define ADD_COMPONENT_POPUP_MENU_ITEM(component) \
			if (!m_SelectedEntity.HasComponent<component>() && ImGui::MenuItem(#component)) \
				m_SelectedEntity.AddComponent<component>()

			ADD_COMPONENT_POPUP_MENU_ITEM(TransformComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(SpriteComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(ResizableSpriteComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(CameraComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(RigidbodyComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(BoxColliderComponent);

			// Scripts list
			ImGui::Separator();
			if (ImGui::BeginMenu("Script"))
			{
				for (auto& [scriptName, addScriptFunction] : ScriptFactory::Get().m_ScriptRegistry)
				{
					if (ImGui::MenuItem(scriptName.c_str()))
						addScriptFunction(m_SelectedEntity);
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::ArrowButton("##down", ImGuiDir_Down))
		{
			ImGui::OpenPopup("Entity options");
		}

		if (ImGui::BeginPopup("Entity options")) {
			if (ImGui::MenuItem("Create prefab"))
			{
				PrefabManager::CreatePrefabFromEntity(m_SelectedEntity);
			}
			if (ImGui::MenuItem("Delete entity"))
			{
				m_SelectedEntity.Destroy();
				EditorLayer::SelectEntity({});
				ImGui::EndPopup();
				ImGui::End();
				return;
			}
			ImGui::EndPopup();
		}
		ImGui::Columns(1);

		ImGui::Dummy({ 0, 5 });
		ImGui::Separator();
		ImGui::Dummy({0, 5});

		// ******************************************************
		// Tag Component UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<TagComponent>())
		{
			DrawComponentUI<TagComponent>("Tag", [](auto& component)
			{
				char buffer[256];
				strcpy_s(buffer, sizeof(buffer), component.Tag.c_str());
				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
					component.Tag = std::string(buffer);
			});
		}

		// ******************************************************
		// Transform Component UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<TransformComponent>())
		{
			DrawComponentUI<TransformComponent>("Transform", [&](auto& component)
			{
				// Posittion
				ImGui::Columns(2); ImGui::SetColumnWidth(0, 75.0f);
				ImGui::Text("Position");
				ImGui::NextColumn();
				ImGui::PushItemWidth(75.0f);
				ImGui::DragFloat("##P_X", &component.Position.x, 0.01f, 0.0f, 0.0f, "%.3f");
				ImGui::SameLine();
				ImGui::PushItemWidth(75.0f);
				ImGui::DragFloat("##P_Y", &component.Position.y, 0.01f, 0.0f, 0.0f, " %.3f");
				ImGui::SameLine();
				ImGui::PushItemWidth(75.0f);
				ImGui::DragFloat("##P_Z", &component.Position.z, 0.0001f, 0.0f, 0.0f, "%.3f");
				ImGui::Columns(1);

				// Scale
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 75.0f);
				ImGui::Text("Scale");
				ImGui::NextColumn();
				ImGui::PushItemWidth(75.0f);
				if (ImGui::DragFloat("##S_X", &component.Scale.x, 0.01f, 0.0f, 0.0f, "%.3f"))
				{
					if (m_SelectedEntity.HasComponent<ResizableSpriteComponent>())
					{
						auto& nsc = m_SelectedEntity.GetComponent<ResizableSpriteComponent>();
						nsc.ResizableSprite.Generate();
					}
				}
				ImGui::SameLine();
				ImGui::PushItemWidth(75.0f);
				if(ImGui::DragFloat("##S_Y", &component.Scale.y, 0.01f, 0.0f, 0.0f, "%.3f"))
				{
					if (m_SelectedEntity.HasComponent<ResizableSpriteComponent>())
					{
						auto& nsc = m_SelectedEntity.GetComponent<ResizableSpriteComponent>();
						nsc.ResizableSprite.Generate();
					}
				}
				ImGui::Columns(1);

				// Rotation 
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 75.0f);
				ImGui::Text("Rotation");
				ImGui::NextColumn();
				ImGui::PushItemWidth(75.0f);
				ImGui::DragFloat("##R", &component.Rotation, 0.2f, 0.0f, 0.0f, "%.3f");

				ImGui::Columns(1);
			});
		}

		// ******************************************************
		// Sprite Component UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<SpriteComponent>())
		{
			DrawComponentUI<SpriteComponent>("Sprite", [&](auto& component)
			{
				Sprite& sprite = component.Sprite;
				std::string textureFilename = sprite ? sprite.GetTexture()->GetPath() : "Fill color";

				// Select texture
				ImGui::Text("Texture:");
				ImGui::PushItemWidth(200.0f);
				if (ImGui::BeginCombo("##sprite_comp_select_texture", textureFilename.c_str()))
				{
					if (ImGui::Selectable("Fill color"))
						sprite.SetTexture(nullptr);

					// Spritesheets
					for (auto& kv : AssetManager::s_Instance->m_SpritesheetList)
					{
						bool isSelected = kv.first == textureFilename;
						ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.9f, 0.3f, 1.0f });
						if (ImGui::Selectable(kv.first.c_str(), isSelected))
						{
							auto& spritesheet = AssetManager::GetSpritesheet(kv.first);
							if (spritesheet)
							{
								sprite = Sprite(spritesheet);
								auto& scale = m_SelectedEntity.GetTransform().Scale;
								float ratio = sprite.GetAspectRatio();
								if (scale.x / scale.y != ratio)
									scale.x = scale.y * ratio;
							}
						}
						ImGui::PopStyleColor();
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					// Textures
					for (auto& path : AssetManager::s_Instance->m_TexturesFilepathList)
					{
						bool isSelected = path == textureFilename;
						ImGui::PushStyleColor(ImGuiCol_Text, { 0.9f, 0.8f, 0.1f, 1.0f });
						if (ImGui::Selectable(path.c_str(), isSelected))
						{
							auto& texture = AssetManager::GetTexture(path);
							if (texture)
							{
								sprite = Sprite(texture);
								auto& scale = m_SelectedEntity.GetTransform().Scale;
								float ratio = sprite.GetAspectRatio();
								if (scale.x / scale.y != ratio)
									scale.x = scale.y * ratio;
							}
						}
						ImGui::PopStyleColor();
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if (ImGui::IsItemClicked())
					AssetManager::ReloadAssetsList();
				ImGui::PopItemWidth();

				ImGui::Dummy({ 0.0f, 5.0f });

				// Tint color control
				ImGui::Text("Tint color:");
				ImGui::Dummy({ 0.0f, 3.0f });
				ImGui::PushItemWidth(260.0f);
				ImGui::ColorEdit4("##Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_AlphaBar);
				ImGui::PopItemWidth();
				ImGui::Dummy({ 0.0f, 5.0f });

				if (sprite)
				{
					// Texture filter mode
					ImGui::Text("Filter mode:"); ImGui::SameLine();
					uint32_t filterMode = (uint32_t)sprite.GetTexture()->GetFilterMode();
					const char* filterModes[] = { "Nearest", "Linear" };

					if (ImGui::BeginCombo("##Texture_Filter", filterModes[filterMode]))
					{
						for (uint32_t i = 0; i < 2; i++)
						{
							const bool isSelected = (filterMode == i);
							if (ImGui::Selectable(filterModes[i], isSelected) && filterMode != i)
							{
								sprite.GetTexture()->SetFilterMode((TextureFilterMode)i);
								filterMode = i;
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					// Mirror flip
					ImGui::Dummy({ 0.0f, 5.0f });
					ImGui::Text("Mirror flip: ");
					ImGui::SameLine();
					ImGui::Checkbox("X##Flip", &sprite.m_MirrorFlipX);
					ImGui::SameLine();
					ImGui::Checkbox("Y##Flip", &sprite.m_MirrorFlipY);
					ImGui::Dummy({ 0.0f, 5.0f });

					// Tiling factor
					ImGui::DragFloat("Tiling factor", &component.TilingFactor, 0.1f);
					ImGui::Dummy({ 0.0f, 5.0f });
				}

				// Check if texture is spritesheet
				if (sprite && sprite.m_Spritesheet)
				{
					ImGui::Dummy({ 0, 5 });
					ImGui::Separator();
					ImGui::Text("Spritesheet proporties");
					ImGui::Dummy({ 0, 5 });

					glm::ivec2 tilePos = (glm::ivec2)sprite.m_TilePos;
					if (ImGui::DragInt2("Tile coords", glm::value_ptr(tilePos), 1, 0))
					{
						if (tilePos.x >= 0 && tilePos.y >= 0)
							sprite.SetTile(tilePos.x, tilePos.y);
					}

					glm::ivec2 tileSize = (glm::ivec2)sprite.m_TileSize;
					if (ImGui::DragInt2("Size", glm::value_ptr(tileSize), 0.2f, 1))
					{
						if (tileSize.x > 0 && tileSize.y > 0)
							sprite.SetTileSize((uint32_t)tileSize.x, (uint32_t)tileSize.y);
					}
				}
			});
		}

		// ******************************************************
		// ResizableSpriteComponent UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<ResizableSpriteComponent>())
		{
			DrawComponentUI<ResizableSpriteComponent>("ResizableSprite", [&](auto& component)
				{
					auto& spritesheet = component.ResizableSprite.m_Spritesheet;
					auto& sprite = component.ResizableSprite;
					std::string filename = spritesheet ? spritesheet->GetTexture()->GetPath() : "Select...";

					// Select spritesheet
					ImGui::Text("Spritesheet:");
					if (ImGui::BeginCombo("##source_select", filename.c_str()))
					{
						for (auto& kv : AssetManager::s_Instance->m_SpritesheetList)
						{
							bool isSelected = filename == kv.first;

							if (ImGui::Selectable(kv.first.c_str(), isSelected))
							{
								spritesheet = AssetManager::GetSpritesheet(kv.first);
								sprite.SetSpritesheet(spritesheet);
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					if (ImGui::IsItemClicked())
						AssetManager::ReloadAssetsList();

					ImGui::Dummy({ 0.0f, 5.0f });
					float tileScale = sprite.m_TileScale;
					if (ImGui::DragFloat("Tile scale", &tileScale, 0.001f))
					{
						sprite.SetTileScale(tileScale);
					}
					ImGui::DragInt2("Spritesheet offset", (int*)glm::value_ptr(sprite.m_PositionOffset));
					ImGui::Dummy({ 0.0f, 5.0f });

					bool left        = sprite.m_Edges & (1 << 0);
					bool right       = sprite.m_Edges & (1 << 1);
					bool top         = sprite.m_Edges & (1 << 2);
					bool bottom      = sprite.m_Edges & (1 << 3);
					bool topLeft     = sprite.m_Edges & (1 << 4);
					bool topRight    = sprite.m_Edges & (1 << 5);
					bool bottomLeft  = sprite.m_Edges & (1 << 6);
					bool bottomRight = sprite.m_Edges & (1 << 7);

					// Toggle sprite edges texture
					ImGui::Text("Toggle sprite edges texture:");
					ImGui::Checkbox("##tb_top_left_corner", &topLeft);
					ImGui::SameLine();
					ImGui::Checkbox("##tb_top_edge", &top);
					ImGui::SameLine();
					ImGui::Checkbox("##tb_top_right_corner", &topRight);
							
					ImGui::Checkbox("##tb_left_edge", &left);
					ImGui::SameLine(); ImGui::Dummy({ 24.0f, 0.0f }); ImGui::SameLine();
					ImGui::Checkbox("##tb_right_edge", &right);

					ImGui::Checkbox("##tb_bottom_left_corner", &bottomLeft);
					ImGui::SameLine();
					ImGui::Checkbox("##tb_bottom_edge", &bottom);
					ImGui::SameLine(); 
					ImGui::Checkbox("##tb_bottom_right_corner", &bottomRight);

					sprite.SetEdges(left * 1 + right * 2 + top * 4 + bottom * 8
						+ topLeft * 16 + topRight * 32 + bottomLeft * 64 + bottomRight * 128);

					ImGui::Dummy({ 0, 3.0f });

					// Tint color control
					ImGui::Text("Tint color:");
					ImGui::Dummy({ 0.0f, 3.0f });
					ImGui::PushItemWidth(260.0f);
					ImGui::ColorEdit4("##Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_AlphaBar);
					ImGui::PopItemWidth();
					ImGui::Dummy({ 0.0f, 10.0f });
			});
		}

		// ******************************************************
		// CameraComponent UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<CameraComponent>())
		{
			DrawComponentUI<CameraComponent>("Camera", [&](auto& component)
				{
					bool isPrimary = m_ActiveScene->m_PrimaryCameraEntity == m_SelectedEntity.m_Handle;
					if (ImGui::Checkbox("Set as primary", &isPrimary))
					{
						if (isPrimary)
							m_ActiveScene->SetPrimaryCameraEntity(m_SelectedEntity);
						else
							m_ActiveScene->SetPrimaryCameraEntity(Entity{});
					}

					float zoom = component.Camera.GetZoomLevel();
					if (ImGui::DragFloat("Zoom level", &zoom, 0.01f))
						component.Camera.SetZoomLevel(zoom);
					ImGui::DragFloat2("Position offset", glm::value_ptr(component.PositionOffset), 0.01f);
				});
		}

		// ******************************************************
		// RigidbodyComponent UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<RigidbodyComponent>())
		{
			DrawComponentUI<RigidbodyComponent>("Rigidbody", [](auto& component)
			{
				std::string bodyType = "Static";
				if (component.Type == b2_dynamicBody)
					bodyType = "Dynamic";
				else if (component.Type == b2_kinematicBody)
					bodyType = "Kinematic";

				if (ImGui::BeginCombo("Body type", bodyType.c_str()))
				{
					if (ImGui::Selectable("Static"))
						component.Type = b2_staticBody;
					else if (ImGui::Selectable("Dynamic"))
						component.Type = b2_dynamicBody;
					else if (ImGui::Selectable("Kinematic"))
						component.Type = b2_kinematicBody;

					ImGui::EndCombo();
				}

				ImGui::Dummy({ 0.0f, 3.0f });
				ImGui::Checkbox("Fixed rotation", &component.FixedRotation);
			});
		}

		// ******************************************************
		// BoxColliderComponent UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<BoxColliderComponent>())
		{
			DrawComponentUI<BoxColliderComponent>("BoxCollider", [](auto& component)
			{
				ImGui::Dummy({ 0.0f, 5.0f });
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size), 0.01f);
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), 0.01f);
				ImGui::DragFloat("Friction", &component.Material.Friction, 0.01f);
				ImGui::DragFloat("Restitution", &component.Material.Restitution, 0.01f);
				ImGui::DragFloat("RestitutionThreshold", &component.Material.RestitutionThreshold, 0.01f);
				ImGui::DragFloat("Density", &component.Material.Density, 0.01f);
				ImGui::Checkbox("IsSensor", &component.IsSensor);
			});
		}

		// ******************************************************
		// Script Component UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<ScriptComponent>())
		{
			auto& component = m_SelectedEntity.GetComponent<ScriptComponent>();
			for (auto& [scriptClassName, scriptInstance] : component.Scripts)
			{
				if (!scriptInstance)
					continue;
							
				ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth
					| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

				std::string title = scriptClassName + " (Script)";
				bool opened = ImGui::TreeNodeEx(scriptClassName.c_str(), treeNodeFlags, title.c_str());

				ImGui::SameLine(ImGui::GetWindowWidth() - 90.0f);
				bool removeScript = ImGui::Button(("Remove##" + scriptClassName).c_str());

				if (opened)
				{
					ImGui::Dummy({ 0.0f, 3.0f });
					for (auto& [fieldName, fieldData] : scriptInstance->m_ScriptFields)
					{
						switch (fieldData.Type)
						{
						case ScriptFieldType::Float:
							ImGui::DragFloat(fieldName.c_str(), (float*)fieldData.InstanceFieldValue, 0.01f);
							break;
						case ScriptFieldType::Float2:
							ImGui::DragFloat2(fieldName.c_str(), (float*)fieldData.InstanceFieldValue, 0.01f);
							break;
						case ScriptFieldType::Float3:
							ImGui::DragFloat3(fieldName.c_str(), (float*)fieldData.InstanceFieldValue, 0.01f);
							break;
						case ScriptFieldType::Float4:
							if (fieldName.find("Color") != fieldName.npos || fieldName.find("color") != fieldName.npos)
								ImGui::ColorEdit4(fieldName.c_str(), (float*)fieldData.InstanceFieldValue);
							else
								ImGui::DragFloat4(fieldName.c_str(), (float*)fieldData.InstanceFieldValue, 0.01f);
							break;

						case ScriptFieldType::Int:
							ImGui::DragInt(fieldName.c_str(), (int*)fieldData.InstanceFieldValue);
							break;
						case ScriptFieldType::Int2:
							ImGui::DragInt2(fieldName.c_str(), (int*)fieldData.InstanceFieldValue);
							break;
						case ScriptFieldType::Int3:
							ImGui::DragInt3(fieldName.c_str(), (int*)fieldData.InstanceFieldValue);
							break;
						case ScriptFieldType::Int4:
							ImGui::DragInt4(fieldName.c_str(), (int*)fieldData.InstanceFieldValue);
							break;

						case ScriptFieldType::Bool:
							ImGui::Checkbox(fieldName.c_str(), (bool*)fieldData.InstanceFieldValue);
							break;
						}
					}
					ImGui::TreePop();
				}

				if (removeScript)
				{
					bool breakLoop = scriptInstance->m_ScriptFields.size() == 1;
					m_SelectedEntity.RemoveScript(scriptClassName);
					if (breakLoop)
						break;
				}
				ImGui::Dummy({ 0.0f, 10.0f });
			}
		}

		ImGui::End();
	}


	void InspectorPanel::DrawSceneProporties()
	{
		ImGui::Text("Scene proporties");
		ImGui::Separator();
		ImGui::Dummy({ 0.0f, 3.0f });

		// Scene name
		ImGui::Text("Scene name");
		strcpy_s(m_SceneNameBuffer, m_ActiveScene->m_SceneName.c_str());
		if (ImGui::InputText("##scene_name", m_SceneNameBuffer, 256))
			m_ActiveScene->m_SceneName = m_SceneNameBuffer;

		// Screen clear color
		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Text("Screen clear color");
		if (ImGui::ColorEdit4("##screen_clear_color", glm::value_ptr(m_ActiveScene->m_ClearColor)))
			Renderer::SetClearColor(m_ActiveScene->m_ClearColor);
		ImGui::Dummy({ 0.0f, 5.0f });

		// Physics configuration
		ImGui::Text("Physics settings");
		ImGui::Dummy({ 0.0f, 3.0f });
		ImGui::Separator();
		bool enablePhysics = m_ActiveScene->m_EnablePhysics;
		if (ImGui::Checkbox("Enable Physics", &enablePhysics) && m_ActiveScene->m_SceneState == SceneState::Stop)
			m_ActiveScene->m_EnablePhysics = enablePhysics;
		if (m_ActiveScene->m_EnablePhysics) 
		{
			ImGui::Dummy({ 0,5 });
			ImGui::PushItemWidth(100.0f);
			ImGui::DragFloat("World gravity", &m_ActiveScene->m_PhysicsWorld->m_Gravity, 0.1f);

			int* vi = &m_ActiveScene->m_PhysicsWorld->m_PhysicsVelocityIterations;
			int* pi = &m_ActiveScene->m_PhysicsWorld->m_PhysicsPositionIterations;
			if (ImGui::DragInt("Velocity iterations", vi))
				*vi = glm::max(*vi, 1);
			if (ImGui::DragInt("Position iterations", pi))
				*pi = glm::max(*pi, 1);

			ImGui::PopItemWidth();
		}
	}


	template<typename T>
	void InspectorPanel::DrawComponentUI(const std::string& name, const std::function<void(T&)>& drawContentFunction)
	{
		T& component = m_SelectedEntity.GetComponent<T>();

		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;
		

		bool opened = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());

		bool removeComponent = false;
		if (!std::is_same<T, TagComponent>::value && !std::is_same<T, TransformComponent>::value)
		{
			ImGui::SameLine(ImGui::GetWindowWidth() - 90.0f);
			removeComponent = ImGui::Button(("Remove##" + name).c_str());
		}

		if (opened)
		{
			ImGui::Dummy({ 0.0f, 3.0f });
			drawContentFunction(component);
			ImGui::TreePop();
		}

		if (removeComponent)
			m_SelectedEntity.RemoveComponent<T>();

		ImGui::Dummy({ 0.0f, 3.0f });
	}

}

#endif // PT_EDITOR
