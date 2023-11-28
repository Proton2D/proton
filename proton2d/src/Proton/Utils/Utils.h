#pragma once 

#include <string>
#include <fstream>
#include <glm/glm.hpp>

namespace proton { 
	
	namespace Utils {
		std::string ReadFile(const std::string& filepath);

		std::vector<std::string> ScanDirectory(const std::string& directory,
			std::initializer_list<std::string> extensionsFilter = {}, bool returnExtensions = true);

		std::vector<std::string> ScanDirectoryRecursive(const std::string& directory,
			std::initializer_list<std::string> extensionsFilter = {}, bool returnExtensions = true);
	} 

	namespace FileDialogs {

		// Implementation per platform
		std::string OpenFile(const char* filter);
		std::string SaveFile(const char* filter);

	}

	namespace Math {

		glm::mat4 GetTransform(const glm::vec3& position, const glm::vec2& scale, float rotation = 0.0f);

		bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	}
}
