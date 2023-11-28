#pragma once
#include <nlohmann/json.hpp>

namespace proton {

	using json = nlohmann::ordered_json;

	class Scene;
	class Entity;

	class SceneSerializer
	{
	public:
		SceneSerializer(Scene* scene);
		~SceneSerializer() = default;

		bool Serialize(const std::string& filepath);
		json SerializeEntity(Entity entity, bool serializeUUID = true);

		bool Deserialize(const std::string& filepath);
		Entity DeserializeEntity(json jsonObj, bool deserializeUUID = true);
	
	private:
		Scene* m_Scene;
	};

}
