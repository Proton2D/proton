#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/SceneHierarchyPanel.h"
#include "Proton/Editor/EditorLayer.h"

#include <imgui.h>


namespace proton {

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");
		if (!m_ActiveScene)
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (ImGui::AcceptDragDropPayload("Entity"))
				m_EntityDragTarget.PopHierarchy();
			ImGui::EndDragDropTarget();
		}

		// TODO: Change to Scene::GetEntitiesWithComponents
		m_ActiveScene->m_Registry.view<RelationshipComponent>().each(
			[&](entt::entity id, auto& relationship)
			{
				if (relationship.Parent == entt::null)
					DrawEntityTreeNode(Entity{ id, m_ActiveScene });
			});


		static Entity treeNodeHovered; // persist state
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
		{
			treeNodeHovered = m_TreeNodeHovered;
			ImGui::OpenPopup("hierarchy_popup");
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !m_TreeNodeHovered)
			EditorLayer::SelectEntity({});

		if (ImGui::BeginPopup("hierarchy_popup"))
		{
			if (!treeNodeHovered)
			{
				if (ImGui::MenuItem("Create Entity"))
					EditorLayer::SelectEntity(m_ActiveScene->CreateEntity());
			}
			else
			{
				if (ImGui::MenuItem("Create Child Entity"))
					EditorLayer::SelectEntity(treeNodeHovered.CreateChildEntity("Entity"));
			}
			ImGui::EndPopup();
		}
		
		m_TreeNodeHovered = Entity{};
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

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
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

		if (ImGui::IsItemHovered())
			m_TreeNodeHovered = entity;

		if (ImGui::IsItemClicked())
			EditorLayer::SelectEntity(entity);

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
