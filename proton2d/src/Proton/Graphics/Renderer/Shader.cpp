//
// This file provides functionalities for managing and compiling shaders in OpenGL.
// From Hazel Engine Renderer OpenGL API:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Platform/OpenGL/OpenGLShader.cpp
//
#include "ptpch.h"
#include "Proton/Graphics/Renderer/Shader.h"
#include "Proton/Utils/Utils.h"

#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace proton {

	Shader::Shader(const std::string& filePath)
		: m_Name(std::filesystem::path(filePath).stem().string())
	{
		std::string vertexSource = Utils::ReadFile(filePath + ".vert");
		std::string fragmentSource = Utils::ReadFile(filePath + ".frag");

		Compile({ {GL_VERTEX_SHADER, vertexSource}, {GL_FRAGMENT_SHADER, fragmentSource} });
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_Object_ID);
	}

	void Shader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;

		for (auto& [type, source] : shaderSources)
		{
			GLuint shader = glCreateShader(type);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				PT_CORE_ERROR(infoLog.data());
				PT_CORE_ASSERT(false, "Shader compilation failure!");
				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		m_Object_ID = program;

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);

			for (auto id : glShaderIDs)
				glDeleteShader(id);

			PT_CORE_ERROR(infoLog.data());
			PT_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		for (auto id : glShaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
	}

	void Shader::Bind() const
	{
		glUseProgram(m_Object_ID);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	void Shader::SetInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_Object_ID, name.c_str());
		glUniform1i(location, value);
	}

	void Shader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_Object_ID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void Shader::SetFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_Object_ID, name.c_str());
		glUniform1f(location, value);
	}

	void Shader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_Object_ID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void Shader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_Object_ID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void Shader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_Object_ID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void Shader::SetMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_Object_ID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::SetMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_Object_ID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}
