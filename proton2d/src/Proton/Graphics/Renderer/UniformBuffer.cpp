//
// This file provides functionalities for creation, binding, and data handling of OpenGL uniform buffers.
// From Hazel Renderer OpenGL API: 
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Platform/OpenGL/OpenGLUniformBuffer.cpp
//
#include "ptpch.h"
#include "Proton/Graphics/Renderer/UniformBuffer.h"

#include <glad/glad.h>

namespace proton {

	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &m_Object_ID);
		glNamedBufferData(m_Object_ID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_Object_ID);
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &m_Object_ID);
	}

	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_Object_ID, offset, size, data);
	}

}
