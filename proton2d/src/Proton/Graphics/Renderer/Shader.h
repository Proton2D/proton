//
// This file provides functionalities for managing and compiling shaders in OpenGL.
// From Hazel Engine Renderer OpenGL API:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Platform/OpenGL/OpenGLShader.h
//
#pragma once
#include <glm/glm.hpp>
#include <unordered_map>

typedef unsigned int GLenum;

namespace proton {

	class Shader
	{
	public:
		Shader(const std::string& filePath);
		virtual ~Shader();

		void Bind() const;
		void Unbind() const;

		void SetInt(const std::string& name, int value);
		void SetIntArray(const std::string& name, int* values, uint32_t count);
		void SetFloat(const std::string& name, float value);
		void SetFloat2(const std::string& name, const glm::vec2& value);
		void SetFloat3(const std::string& name, const glm::vec3& value);
		void SetFloat4(const std::string& name, const glm::vec4& value);
		void SetMat3(const std::string& name, const glm::mat3& value);
		void SetMat4(const std::string& name, const glm::mat4& value);

		const std::string& GetName() const { return m_Name; }
	
	private:
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	
	private:
		uint32_t m_Object_ID;
		std::string m_Name;
	};

}
