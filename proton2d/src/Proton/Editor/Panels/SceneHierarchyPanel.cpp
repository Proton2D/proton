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

		ImGui::Dummy(ImGui::GetWindowSize());
		ImGui::SetCursorPos({ 0, 25 });
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ENTITY"))
			{
				Entity dragTarget = *(Entity*)payload->Data;
				dragTarget.PopHierarchy();
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::Dummy({ 0, 2 });


		for (Entity entity : m_ActiveScene->m_Root)
		{
			DrawEntityTreeNode(entity);
		}

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
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("SCENE_HIERARCHY_ENTITY", (void*)&entity, sizeof(Entity));
				ImGui::EndDragDropSource();
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ENTITY"))
			{
				Entity dragTarget = *(Entity*)payload->Data;
				if (!dragTarget.IsParentOf(entity))
				{
					dragTarget.PopHierarchy();
					entity.AddChildEntity(dragTarget);
				}
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
