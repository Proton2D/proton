#include "ptpch.h"
#include "Proton/Utils/Utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <filesystem>

#ifdef PROTON_PLATFORM_WINDOWS
constexpr const char DIR_SLASH = '\\';
#else
constexpr const char DIR_SLASH = '/';
#endif


namespace proton { namespace Utils {

	std::string ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
				in.close();
			}
			else
				PT_CORE_ERROR("[Utils::ReadFile] Could not read from file '{}'", filepath);
		}
		else
			PT_CORE_ERROR("[Utils::ReadFile] Could not open file '{}'", filepath);

		return result;
	}

	std::vector<std::string> ScanDirectory(const std::string& directory,
		std::initializer_list<std::string> extensionFilter, bool returnWithExtensions)
	{
		std::vector<std::string> files;
		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			std::string filepath = entry.path().string();
			std::string extension = filepath.substr(filepath.find("."));
			for (const std::string& ext : extensionFilter)
			{
				if (extension == ext)
				{
					if (!returnWithExtensions)
						filepath = filepath.substr(0, filepath.size() - extension.size());
					files.push_back(filepath.substr(filepath.find_first_of(DIR_SLASH) + 1));
					break;
				}
			}
		}
		return files;
	}

	std::vector<std::string> ScanDirectoryRecursive(const std::string& directory,
		std::initializer_list<std::string> extensionFilter, bool returnWithExtensions)
	{
		std::vector<std::string> files;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
			if (entry.is_regular_file()) 
			{
				std::string filepath = entry.path().string();
				std::string extension = filepath.substr(filepath.find("."));
				for (const std::string& ext : extensionFilter)
				{
					if (extension == ext) 
					{
						if (!returnWithExtensions)
							filepath = filepath.substr(0, filepath.size() - extension.size());
						files.push_back(filepath.substr(filepath.find_first_of(DIR_SLASH) + 1));
						break;
					}
				}
			}
		}
		return files;
	}

} 

namespace Math {

	glm::mat4 GetTransform(const glm::vec3& position, const glm::vec2& scale, float rotation)
	{
		return glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });
	}


	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}


		return true;
	}
}
}