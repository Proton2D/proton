#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/InspectorPanel.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Graphics/Renderer/Renderer.h"
#include "Proton/Utils/Utils.h"
#include "Proton/Assets/AssetManager.h"
#include "Proton/Scripting/ScriptFactory.h"
#include "Proton/Scene/EntityComponent.h"
#include "Proton/Scripting/EntityScript.h"
#include "Proton/Scene/PrefabManager.h"
#include "Proton/Physics/PhysicsWorld.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace proton {

	static const std::string s_TexturesPath = "content/textures/";

	static std::string GetFilepathRelative(const std::string& parentDir, const std::string& fullFilepath)
	{
		return fullFilepath.substr(parentDir.size(), fullFilepath.size() - parentDir.size());;
	}

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

		char buffer[256];
		strcpy_s(buffer, sizeof(buffer), m_SelectedEntity.GetTag().c_str());
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 5 });
		if (ImGui::InputText("##tag", buffer, sizeof(buffer)))
			m_SelectedEntity.GetComponent<TagComponent>().Tag = std::string(buffer);
		ImGui::PopStyleVar();
		ImGui::SameLine();

		// Add component popup
		ImGui::NextColumn();
		ImGui::PushFont(EditorLayer::GetFontAwesome());
		if (ImGui::Button(u8"\uF067", { 42, 28 }))
			ImGui::OpenPopup("Add component");
		ImGui::PopFont();

		if (ImGui::BeginPopup("Add component"))
		{
			#define ADD_COMPONENT_POPUP_MENU_ITEM(component) \
			if (!m_SelectedEntity.HasComponent<component>() && ImGui::MenuItem(#component)) \
				m_SelectedEntity.AddComponent<component>()

			ADD_COMPONENT_POPUP_MENU_ITEM(TransformComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(SpriteComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(ResizableSpriteComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(CircleRendererComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(CameraComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(RigidbodyComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(BoxColliderComponent);
			ADD_COMPONENT_POPUP_MENU_ITEM(CircleColliderComponent);

			// Scripts list
			ImGui::Separator();
			if (ImGui::BeginMenu("Script"))
			{
				for (auto& [scriptName, addScriptFunction] : ScriptFactory::Get().m_ScriptRegistry)
				{
					if (m_SelectedEntity.HasComponent<ScriptComponent>())
					{
						auto& component = m_SelectedEntity.GetComponent<ScriptComponent>();
						if (component.Scripts.find(scriptName) != component.Scripts.end())
							continue;
					}
					if (ImGui::MenuItem(scriptName.c_str()))
						addScriptFunction(m_SelectedEntity);
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::PushFont(EditorLayer::GetFontAwesome());
		if (ImGui::Button(u8"\uF141", { 42, 28 }))
		{
			ImGui::OpenPopup("Entity options");
		}
		ImGui::PopFont();

		if (ImGui::BeginPopup("Entity options")) {
			if (ImGui::MenuItem("Create Prefab"))
			{
				PrefabManager::CreatePrefabFromEntity(m_SelectedEntity);
			}
			if (ImGui::MenuItem("Delete Entity"))
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
				ImGui::DragFloat("##P_X", &component.LocalPosition.x, 0.01f, 0.0f, 0.0f, "%.3f");
				ImGui::SameLine();
				ImGui::PushItemWidth(75.0f);
				ImGui::DragFloat("##P_Y", &component.LocalPosition.y, 0.01f, 0.0f, 0.0f, " %.3f");
				ImGui::SameLine();
				ImGui::PushItemWidth(75.0f);
				ImGui::DragFloat("##P_Z", &component.LocalPosition.z, 0.0001f, 0.0f, 0.0f, "%.3f");
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
						nsc.ResizableSprite.Generate(&m_SelectedEntity);
					}
				}
				ImGui::SameLine();
				ImGui::PushItemWidth(75.0f);
				if(ImGui::DragFloat("##S_Y", &component.Scale.y, 0.01f, 0.0f, 0.0f, "%.3f"))
				{
					if (m_SelectedEntity.HasComponent<ResizableSpriteComponent>())
					{
						auto& nsc = m_SelectedEntity.GetComponent<ResizableSpriteComponent>();
						nsc.ResizableSprite.Generate(&m_SelectedEntity);
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
				std::string textureFilename = sprite 
					? GetFilepathRelative(s_TexturesPath, sprite.GetTexture()->GetPath())
					: "Fill color";

				// Select texture
				if (ImGui::BeginCombo("Texture", textureFilename.c_str()))
				{
					if (ImGui::Selectable("Fill Color"))
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

				// Tint color control
				ImGui::Dummy({ 0, 2 });
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_AlphaBar);
				ImGui::Dummy({ 0, 2 });

				if (sprite)
				{
					// Mirror flip
					ImGui::Text("Mirror Flip");
					ImGui::SameLine();
					ImGui::Checkbox("X##Flip", &sprite.m_MirrorFlipX);
					ImGui::SameLine();
					ImGui::Checkbox("Y##Flip", &sprite.m_MirrorFlipY);
					ImGui::Dummy({ 0, 1 });

					// Texture filter mode
					uint32_t filterMode = (uint32_t)sprite.GetTexture()->GetFilterMode();
					const char* filterModes[] = { "Nearest", "Linear" };

					if (ImGui::BeginCombo("Filter Mode", filterModes[filterMode]))
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

					// Tiling factor
					ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f);
				}

				// Check if texture is spritesheet
				if (sprite && sprite.m_Spritesheet)
				{
					ImGui::Dummy({ 0, 5 });
					ImGui::Separator();
					ImGui::Text("Spritesheet");
					ImGui::Dummy({ 0, 3 });

					glm::ivec2 tilePos = (glm::ivec2)sprite.m_TilePos;
					if (ImGui::DragInt2("Tile Coords", glm::value_ptr(tilePos), 1, 0))
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
					std::string filename = spritesheet 
						? GetFilepathRelative(s_TexturesPath, spritesheet->GetTexture()->GetPath())
						: "Select...";

					// Select spritesheet
					if (ImGui::BeginCombo("Spritesheet", filename.c_str()))
					{
						for (auto& kv : AssetManager::s_Instance->m_SpritesheetList)
						{
							bool isSelected = filename == kv.first;

							if (ImGui::Selectable(kv.first.c_str(), isSelected))
							{
								spritesheet = AssetManager::GetSpritesheet(kv.first);
								sprite.SetSpritesheet(spritesheet, &m_SelectedEntity);
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					if (ImGui::IsItemClicked())
						AssetManager::ReloadAssetsList();

					float tileScale = sprite.m_TileScale;
					if (ImGui::DragFloat("Tile Scale", &tileScale, 0.001f))
					{
						sprite.SetTileScale(tileScale, &m_SelectedEntity);
					}
					ImGui::DragInt2("Tile Offset", (int*)glm::value_ptr(sprite.m_PositionOffset));
					ImGui::Dummy({ 0.0f, 3.0f });

					// Toggle sprite edges texture
					unsigned int edges = (unsigned int)sprite.m_Edges;
					ImGui::Text("Toggle edge and corner texture:");
					ImGui::CheckboxFlags("##tb_top_left", &edges, Edge_TopLeft);
					ImGui::SameLine();
					ImGui::CheckboxFlags("##tb_top", &edges, Edge_Top);
					ImGui::SameLine();
					ImGui::CheckboxFlags("##tb_top_right", &edges, Edge_TopRight);
							
					ImGui::CheckboxFlags("##tb_left", &edges, Edge_Left);
					ImGui::SameLine(); 
					ImGui::Dummy({ 24.0f, 0.0f }); 
					ImGui::SameLine();
					ImGui::CheckboxFlags("##tb_right", &edges, Edge_Right);

					ImGui::CheckboxFlags("##tb_bottom_left", &edges, Edge_BottomLeft);
					ImGui::SameLine();
					ImGui::CheckboxFlags("##tb_bottom", &edges, Edge_Bottom);
					ImGui::SameLine(); 
					ImGui::CheckboxFlags("##tb_bottom_right", &edges, Edge_BottomRight);

					sprite.SetEdges((uint8_t)edges, &m_SelectedEntity);
					ImGui::Dummy({ 0, 3.0f });

					// Tint color control
					ImGui::ColorEdit4("Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_AlphaBar);
			});
		}

		// ******************************************************
		// Circle Renderer Component UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<CircleRendererComponent>())
		{
			DrawComponentUI<CircleRendererComponent>("CircleRenderer", [&](auto& component)
				{
					ImGui::ColorEdit4("Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_AlphaBar);
					ImGui::SliderFloat("Thickness", &component.Thickness, 0.0f, 1.0f);
					ImGui::SliderFloat("Fade", &component.Fade, 0.0f, 1.0f);
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
					if (ImGui::Checkbox("Set as primary", &isPrimary) && isPrimary)
						m_ActiveScene->SetPrimaryCameraEntity(m_SelectedEntity);

					float zoom = component.Camera.GetZoomLevel();
					if (ImGui::DragFloat("Zoom Level", &zoom, 0.01f))
						component.Camera.SetZoomLevel(zoom);
					ImGui::DragFloat2("Offset", glm::value_ptr(component.PositionOffset), 0.01f);
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

				if (ImGui::BeginCombo("Body Type", bodyType.c_str()))
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
		// CircleColliderComponent UI
		// ******************************************************
		if (m_SelectedEntity.HasComponent<CircleColliderComponent>())
		{
			DrawComponentUI<CircleColliderComponent>("CircleCollider", [](auto& component)
				{
					ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), 0.01f);
					ImGui::DragFloat("Radius", &component.Radius, 0.001f);
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

				bool keepScript = true;
				bool opened = ImGui::CollapsingHeader((scriptClassName + " (Script)").c_str(), &keepScript, treeNodeFlags);

				if (opened)
				{
					ImGui::Dummy({ 0.0f, 3.0f });
					for (auto& [fieldName, fieldData] : scriptInstance->m_ScriptFields)
					{
						if (!fieldData.ShowInEditor)
							continue;

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
					scriptInstance->OnImGuiRender();
				}

				if (!keepScript)
				{
					m_SelectedEntity.RemoveScript(scriptClassName);
					break;
				}
				ImGui::Dummy({ 0.0f, 10.0f });
			}
		}

		ImGui::End();
	}


	void InspectorPanel::DrawSceneProporties()
	{
		ImGui::Text("Scene Proporties");
		ImGui::Separator();
		ImGui::Dummy({ 0.0f, 3.0f });

		// Scene name
		static char sceneName[256] = {0};
		ImGui::Text("Scene Name");
		strcpy_s(sceneName, m_ActiveScene->m_SceneName.c_str());
		if (ImGui::InputText("##scene_name", sceneName, 256))
			m_ActiveScene->m_SceneName = sceneName;

		// Screen clear color
		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Text("Background Color");
		if (ImGui::ColorEdit4("##screen_clear_color", glm::value_ptr(m_ActiveScene->m_ClearColor)))
			Renderer::SetClearColor(m_ActiveScene->m_ClearColor);
		ImGui::Dummy({ 0.0f, 5.0f });

		// Physics configuration
		ImGui::Text("Physics Settings");
		ImGui::Dummy({ 0.0f, 3.0f });
		ImGui::Separator();
		bool enablePhysics = m_ActiveScene->m_EnablePhysics;
		if (ImGui::Checkbox("Enable Physics", &enablePhysics) && m_ActiveScene->m_SceneState == SceneState::Stop)
			m_ActiveScene->m_EnablePhysics = enablePhysics;
		if (m_ActiveScene->m_EnablePhysics) 
		{
			ImGui::Dummy({ 0,5 });
			ImGui::PushItemWidth(100.0f);
			ImGui::DragFloat("World Gravity", &m_ActiveScene->m_PhysicsWorld->m_Gravity, 0.1f);

			int* vi = &m_ActiveScene->m_PhysicsWorld->m_PhysicsVelocityIterations;
			int* pi = &m_ActiveScene->m_PhysicsWorld->m_PhysicsPositionIterations;
			if (ImGui::DragInt("Velocity Iterations", vi))
				*vi = glm::max(*vi, 1);
			if (ImGui::DragInt("Position Iterations", pi))
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
		
		size_t componentTypeID = typeid(T).hash_code();
		bool canBeRemoved = !std::is_same<T, TagComponent>::value && !std::is_same<T, TransformComponent>::value;
		bool keepComponent = true;
		bool opened = true;

		if (canBeRemoved)
			opened = ImGui::CollapsingHeader((name + "##" + std::to_string(componentTypeID)).c_str(), &keepComponent, treeNodeFlags);
		else
			opened = ImGui::TreeNodeEx((void*)componentTypeID, treeNodeFlags, name.c_str());

		if (opened)
		{
			ImGui::Dummy({ 0.0f, 3.0f });
			drawContentFunction(component);
			if (!canBeRemoved)
				ImGui::TreePop();
		}

		if (!keepComponent)
			m_SelectedEntity.RemoveComponent<T>();

		ImGui::Dummy({ 0.0f, 3.0f });
	}

}

#endif // PT_EDITOR
