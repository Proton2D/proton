#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/SceneHierarchyPanel.h"
#include "Proton/Editor/EditorLayer.h"

#include <imgui.h>


namespace proton {

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");
		ImGui::Dummy({ 0.0f, 1.0f });

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;

		if (!m_SelectedEntity)
			flags |= ImGuiTreeNodeFlags_Selected;

		bool opened = ImGui::TreeNodeEx(m_ActiveScene->m_SceneName.c_str(), flags);

		if (ImGui::IsItemClicked())
			EditorLayer::SelectEntity({});

		if (ImGui::IsItemClicked(1))
			ImGui::OpenPopup("new_entity_root");

		if (ImGui::BeginPopup("new_entity_root"))
		{
			if (ImGui::MenuItem("New entity"))
				m_ActiveScene->CreateEntity();
			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (ImGui::AcceptDragDropPayload("Entity"))
				m_EntityDragTarget.PopHierarchy();
			ImGui::EndDragDropTarget();
		}

		if (opened)
		{
			m_ActiveScene->m_Registry.each([&](auto id)
				{
					Entity entity{ id, m_ActiveScene };
					auto& relationship = entity.GetComponent<RelationshipComponent>();

					if (relationship.Parent == entt::null)
						DrawEntityTreeNode(entity);
				});

			ImGui::TreePop();
		}

		ImGui::End();
	}


	void SceneHierarchyPanel::DrawEntityTreeNode(Entity entity)
	{
		auto& relationship = entity.GetComponent<RelationshipComponent>();

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;

		if (m_SelectedEntity == entity)
			flags |= ImGuiTreeNodeFlags_Selected;

		if (relationship.First == entt::null)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, entity.GetComponent<TagComponent>().Tag.c_str());

		if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(0))
		{
			m_EntityDragTarget = entity;
			ImGui::BeginDragDropSource();
			ImGui::SetDragDropPayload("Entity", (void*)&m_EntityDragTarget, sizeof(Entity));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (m_EntityDragTarget && !m_EntityDragTarget.IsParentOf(entity) && ImGui::AcceptDragDropPayload("Entity"))
			{
				m_EntityDragTarget.PopHierarchy();
				entity.AddChildEntity(m_EntityDragTarget);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked())
			EditorLayer::SelectEntity(entity);

		if (ImGui::IsItemClicked(1))
			ImGui::OpenPopup("new_entity_child");
		if (ImGui::BeginPopup("new_entity_child"))
		{
			if (ImGui::MenuItem("New child entity"))
				entity.AddChildEntity(m_ActiveScene->CreateEntity());
			ImGui::EndPopup();
		}

		if (opened)
		{
			if (relationship.ChildrenCount)
			{
				auto current = relationship.First;
				for (uint32_t i = 0; i < relationship.ChildrenCount; i++)
				{
					Entity e{ current, m_ActiveScene };
					DrawEntityTreeNode(e);
					current = e.GetComponent<RelationshipComponent>().Next;
				}
			}
			ImGui::TreePop();
		}
	}

}
#endif // PT_EDITOR
