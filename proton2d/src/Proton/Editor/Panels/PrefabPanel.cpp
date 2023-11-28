#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/PrefabPanel.h"
#include "Proton/Scene/PrefabManager.h"

#include <imgui.h>

namespace proton {
	
	void PrefabPanel::OnImGuiRender()
	{
		ImGui::Begin("Prefabs");
		ImGui::Dummy({ 0, 1 });
		if (ImGui::Button("Refresh"))
			PrefabManager::ReloadAllPrefabs();

		std::string deletePrefabTag;
		ImGui::Dummy({ 0.0f, 5.0f });
		for (auto& [tag, jsonData] : PrefabManager::s_Instance->m_PrefabsJsonData)
		{
			ImGui::Separator();
			ImGui::Text(tag.c_str());
			ImGui::SameLine(ImGui::GetWindowWidth() - 120);
			if (ImGui::Button(("Spawn##" + tag).c_str(), { 60, 25 }))
			{
				Entity entity = PrefabManager::SpawnPrefab(m_ActiveScene, tag);
				auto& transform = entity.GetComponent<TransformComponent>();
				glm::vec2 cameraPos = m_ActiveScene->GetPrimaryCameraPosition();
				transform.Position.x = cameraPos.x;
				transform.Position.y = cameraPos.y;
			}
			ImGui::SameLine();
			if (ImGui::Button(("X##" + tag).c_str(), { 25, 25 }))
				deletePrefabTag = tag;
		}
		if (deletePrefabTag.size())
			PrefabManager::DeletePrefab(deletePrefabTag);
		ImGui::Separator();
		ImGui::End();
	}
}

#endif // PT_EDITOR
