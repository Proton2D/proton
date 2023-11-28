#include "ptpch.h"
#include "Proton/Scene/PrefabManager.h"
#include "Proton/Assets/SceneSerializer.h"
#include "Proton/Utils/Utils.h"

#include <fstream>
#ifdef PT_EDITOR
#include "Proton/Editor/EditorLayer.h"
#endif

namespace proton {

	PrefabManager* PrefabManager::s_Instance = nullptr;

	void PrefabManager::Init()
	{
		if (!s_Instance)
		{
			s_Instance = new PrefabManager();
			s_Instance->ReloadAllPrefabs();
		}
	}

	void PrefabManager::ReloadAllPrefabs()
	{
		s_Instance->m_PrefabsJsonData.clear();
		for (const auto& prefabFile : Utils::ScanDirectoryRecursive("content/prefabs", { ".prefab.json" }, false))
			LoadPrefab(prefabFile);
	}

	void PrefabManager::CreatePrefabFromEntity(Entity entity)
	{
		SceneSerializer serializer(entity.GetScene());
		json jsonData = serializer.SerializeEntity(entity);
		std::string tag = jsonData["Tag"];
		s_Instance->m_PrefabsJsonData[tag] = jsonData;
		std::ofstream file("content/prefabs/" + tag + ".prefab.json");
		file << jsonData.dump(4);
		file.close();
	}

	bool PrefabManager::LoadPrefab(const std::string& prefabPath)
	{
		std::string rawData = Utils::ReadFile("content/prefabs/" + prefabPath + ".prefab.json");
		if (rawData.size())
		{
			json jsonData = json::parse(rawData);
			if (jsonData.contains("Tag"))
				s_Instance->m_PrefabsJsonData[prefabPath] = jsonData;
			return true;
		}
		return false;
	}

	bool PrefabManager::DeletePrefab(const std::string& prefabPath)
	{
		if (Exists(prefabPath))
		{
			if (remove(("content/prefabs/" + prefabPath + ".prefab.json").c_str()) == 0)
			{
				s_Instance->m_PrefabsJsonData.erase(prefabPath);
				return true;
			}
		}
		return false;
	}

	bool PrefabManager::Exists(const std::string& prefabPath)
	{
		return s_Instance->m_PrefabsJsonData.find(prefabPath) != s_Instance->m_PrefabsJsonData.end();
	}

	Entity PrefabManager::SpawnPrefab(Scene* scene, const std::string& prefabPath)
	{
		PT_ASSERT(Exists(prefabPath), "Prefab not loaded");
		SceneSerializer serializer(scene);
		const json& prefabData = s_Instance->m_PrefabsJsonData.at(prefabPath);
		Entity entity = serializer.DeserializeEntity(prefabData, false);
		auto camera = scene->GetPrimaryCameraPosition();
		auto& transform = entity.GetComponent<TransformComponent>();
		transform.Position.x = camera.x;
		transform.Position.y = camera.y;
		return entity;
	}

}
