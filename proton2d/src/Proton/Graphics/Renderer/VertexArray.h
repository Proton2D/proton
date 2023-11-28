//
// This file provides functionalities for creating, binding, and managing vertex arrays in OpenGL,
// including handling different shader data types and vertex buffer layouts.
// From Hazel Renderer OpenGL API:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Platform/OpenGL/OpenGLVertexArray.h
//
#pragma once

#include "Proton/Core/Base.h"
#include "Proton/Graphics/Renderer/Buffer.h"

namespace proton {

	class VertexArray
	{
	public:
		VertexArray();
		virtual ~VertexArray();

		void Bind() const;
		void Unbind() const;

		void AddVertexBuffer(const Shared<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const Shared<IndexBuffer>& indexBuffer);

		const std::vector<Shared<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		const Shared<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		uint32_t m_Object_ID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<Shared<VertexBuffer>> m_VertexBuffers;
		Shared<IndexBuffer> m_IndexBuffer;
	};

}
