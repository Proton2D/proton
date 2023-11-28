//
// This file provides functionalities for creation, binding, and data handling of OpenGL uniform buffers.
// From Hazel Renderer OpenGL API: 
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Platform/OpenGL/OpenGLUniformBuffer.h
//
#pragma once

namespace proton {

	class UniformBuffer
	{
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		virtual ~UniformBuffer();

		void SetData(const void* data, uint32_t size, uint32_t offset = 0);

	private:
		uint32_t m_Object_ID = 0;
	};
}
