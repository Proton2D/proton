// 
// Basic 2D Batch Renderer
// Architecture and Code based upon Hazel 2D Renderer:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Renderer/Renderer2D.cpp
//
#include "ptpch.h"
#include "Proton/Graphics/Renderer/Renderer.h"

#include "Proton/Graphics/Renderer/Shader.h"
#include "Proton/Graphics/Renderer/UniformBuffer.h"
#include "Proton/Graphics/Renderer/VertexArray.h"
#include "Proton/Graphics/Renderer/Texture.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace proton {

	struct QuadVertex // vertex buffer data
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextureCoords;
		float TextureIndex;
		float TilingFactor;
	};

	struct LineVertex // vertex buffer data
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	static struct RendererData
	{
		uint32_t MaxQuads = 10000;
		uint32_t MaxVertices = MaxQuads * 4;
		uint32_t MaxIndices = MaxQuads * 6;
		uint32_t MaxTextureSlots = 32;

		// Quads OpenGL objects
		Shared<VertexArray> QuadVertexArray;
		Shared<VertexBuffer> QuadVertexBuffer;
		Shared<Shader> QuadShader;

		// Quads VertexBuffer data
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
		uint32_t QuadIndexCount = 0;

		// Lines OpenGL objects
		Shared<VertexArray> LineVertexArray;
		Shared<VertexBuffer> LineVertexBuffer;
		Shared<Shader> LineShader;

		// Lines VertexBuffer data
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;
		uint32_t LineVertexCount = 0;
		float LineWidth = 1.0f;

		// Textures and camera uniform buffer
		std::vector<Shared<Texture>> TextureSlots;
		uint32_t TextureSlotIndex = 1;
		Shared<UniformBuffer> CameraUniformBuffer;

		// Stats
		uint32_t OpenGLDrawCalls = 0;
	} data;

	static void OpenGLMessageCallback(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:          PT_CORE_CRITICAL("[OpenGL Error] {}", message); return;
		case GL_DEBUG_SEVERITY_MEDIUM:        PT_CORE_WARN("[OpenGL Warning] {}", message); return;
		case GL_DEBUG_SEVERITY_LOW:           PT_CORE_WARN("[OpenGL Warning] {}", message); return;
		case GL_DEBUG_SEVERITY_NOTIFICATION:  PT_CORE_INFO("[OpenGL Info] {}", message); return;
		}
	}

	void Renderer::Init()
	{
#	ifdef PROTON_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#	endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (int*)&data.MaxTextureSlots);

		// Create quad vertex buffer
		data.QuadVertexBuffer = MakeShared<VertexBuffer>((uint32_t)(data.MaxVertices * sizeof(QuadVertex)));
		data.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "Position"      },
			{ ShaderDataType::Float4, "Color"         },
			{ ShaderDataType::Float2, "TextureCoords" },
			{ ShaderDataType::Float,  "TextureIndex"  },
			{ ShaderDataType::Float,  "TilingFactor"  }
		});
		data.QuadVertexBufferBase = new QuadVertex[data.MaxVertices];

		// Create quad index buffer data
		uint32_t* indicies = new uint32_t[data.MaxIndices];

		for (uint32_t i = 0; i < data.MaxIndices; i++)
		{
			uint32_t offset = 4 * (i / 6);
			constexpr uint32_t quadIndices[] = { 0, 1, 2, 2, 3, 0 };
			indicies[i] = offset + quadIndices[i % 6];
		}

		// Create quad vertex array
		data.QuadVertexArray = MakeShared<VertexArray>();
		data.QuadVertexArray->AddVertexBuffer(data.QuadVertexBuffer);
		data.QuadVertexArray->SetIndexBuffer(MakeShared<IndexBuffer>(indicies, data.MaxIndices));
		delete[] indicies;

		// Create line vertex buffer and vertex array
		data.LineVertexBuffer = MakeShared<VertexBuffer>(data.MaxVertices * (uint32_t)sizeof(LineVertex));
		data.LineVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "Position" },
			{ ShaderDataType::Float4, "Color"    }
		});
		data.LineVertexBufferBase = new LineVertex[data.MaxVertices];
		data.LineVertexArray = MakeShared<VertexArray>();
		data.LineVertexArray->AddVertexBuffer(data.LineVertexBuffer);

		// Init texture slots vector, shaders and camera uniform buffer
		data.TextureSlots.resize(data.MaxTextureSlots);
		data.TextureSlots[0]     = MakeShared<Texture>(1, 1, true); // white texture
		data.QuadShader          = MakeShared<Shader>("content/shaders/Quad2D.glsl");
		data.LineShader          = MakeShared<Shader>("content/shaders/Line2D.glsl");
		data.CameraUniformBuffer = MakeShared<UniformBuffer>((uint32_t)sizeof(glm::mat4), 0);
	
		SetClearColor(DEFAULT_CLEAR_COLOR);
	}

	void Renderer::Shutdown()
	{
		delete[] data.QuadVertexBufferBase;
		delete[] data.LineVertexBufferBase;
	}

	void Renderer::BeginScene(const Camera& camera, const glm::vec3& position)
	{
		PROFILE_FUNCTION();
		glm::mat4 viewMatrix = glm::inverse(glm::translate(glm::mat4(1.0f), position));
		glm::mat4 viewProjection = camera.GetProjection() * viewMatrix;
		data.CameraUniformBuffer->SetData(&viewProjection, sizeof(glm::mat4));
		data.OpenGLDrawCalls = 0;
		StartBatch();
	}

	void Renderer::EndScene()
	{
		PROFILE_FUNCTION();
		Flush();
	}

	void Renderer::StartBatch()
	{
		data.QuadIndexCount = 0;
		data.QuadVertexBufferPtr = data.QuadVertexBufferBase;

		data.LineVertexCount = 0;
		data.LineVertexBufferPtr = data.LineVertexBufferBase;
		
		data.TextureSlotIndex = 1;
	}

	void Renderer::Flush()
	{
		if (data.QuadIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)data.QuadVertexBufferPtr - (uint8_t*)data.QuadVertexBufferBase);
			data.QuadVertexBuffer->SetData(data.QuadVertexBufferBase, dataSize);

			for (uint32_t i = 0; i < data.TextureSlotIndex; i++)
				data.TextureSlots[i]->Bind(i);

			data.QuadShader->Bind();
			data.QuadVertexArray->Bind();
			glDrawElements(GL_TRIANGLES, data.QuadIndexCount, GL_UNSIGNED_INT, nullptr);
			data.OpenGLDrawCalls++;
		}

		if (data.LineVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)data.LineVertexBufferPtr - (uint8_t*)data.LineVertexBufferBase);
			data.LineVertexBuffer->SetData(data.LineVertexBufferBase, dataSize);

			data.LineShader->Bind();
			data.LineVertexArray->Bind();
			glLineWidth(data.LineWidth);
			glDrawArrays(GL_LINES, 0, data.LineVertexCount);
		}
	}

	void Renderer::NextBatch()
	{
		Flush();
		StartBatch();
	}

	constexpr static glm::vec4 QuadVertexPositions[] = {
		{ -0.5f, -0.5f, 0.0f, 1.0f },
		{  0.5f, -0.5f, 0.0f, 1.0f },
		{  0.5f,  0.5f, 0.0f, 1.0f },
		{ -0.5f,  0.5f, 0.0f, 1.0f }
	};

	void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& color, float tilingFactor)
	{
		PROFILE_FUNCTION();

		if (data.QuadIndexCount >= data.MaxIndices)
			NextBatch();

		constexpr glm::vec2 textureCoords[] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};

		constexpr uint16_t QuadVertexCount = 4;
		for (uint16_t i = 0; i < QuadVertexCount; i++)
		{
			data.QuadVertexBufferPtr->Position = transform * QuadVertexPositions[i];
			data.QuadVertexBufferPtr->Color = color;
			data.QuadVertexBufferPtr->TextureIndex = 0.0f;
			data.QuadVertexBufferPtr->TextureCoords = textureCoords[i];
			data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			data.QuadVertexBufferPtr++;
		}

		data.QuadIndexCount += 6;
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const Sprite& sprite, const glm::vec4& tintColor, float tilingFactor)
	{
		DrawQuad(transform, sprite.GetTexture(), sprite.GetTextureCoords(), tintColor, tilingFactor);
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const Shared<Texture>& texture,
		const TextureCoords& textureCoords, const glm::vec4& tintColor, float tilingFactor)
	{
		PROFILE_FUNCTION();

		if (data.QuadIndexCount >= data.MaxIndices)
			NextBatch();

		uint32_t textureIndex = 0;
		for (uint32_t i = 1; i < data.TextureSlotIndex; i++)
		{
			if (*data.TextureSlots[i] == *texture)
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == 0)
		{
			if (data.TextureSlotIndex >= data.MaxTextureSlots)
				NextBatch();

			textureIndex = data.TextureSlotIndex;
			data.TextureSlots[data.TextureSlotIndex] = texture;
			data.TextureSlotIndex++;
		}

		constexpr uint16_t QuadVertexCount = 4;
		for (uint16_t i = 0; i < QuadVertexCount; i++)
		{
			data.QuadVertexBufferPtr->Position = transform * QuadVertexPositions[i];
			data.QuadVertexBufferPtr->Color = tintColor;
			data.QuadVertexBufferPtr->TextureIndex = static_cast<float>(textureIndex);
			data.QuadVertexBufferPtr->TextureCoords = textureCoords[i];
			data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			data.QuadVertexBufferPtr++;
		}

		data.QuadIndexCount += 6;
	}

	void Renderer::DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color)
	{
		data.LineVertexBufferPtr->Position = p0;
		data.LineVertexBufferPtr->Color = color;
		data.LineVertexBufferPtr++;
		
		data.LineVertexBufferPtr->Position = p1;
		data.LineVertexBufferPtr->Color = color;
		data.LineVertexBufferPtr++;
		
		data.LineVertexCount += 2;
	}

	void Renderer::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
		glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

		DrawLine(p0, p1, color);
		DrawLine(p1, p2, color);
		DrawLine(p2, p3, color);
		DrawLine(p3, p0, color);
	}

	void Renderer::DrawRect(const glm::mat4& transform, const glm::vec4& color)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * QuadVertexPositions[i];

		DrawLine(lineVertices[0], lineVertices[1], color);
		DrawLine(lineVertices[1], lineVertices[2], color);
		DrawLine(lineVertices[2], lineVertices[3], color);
		DrawLine(lineVertices[3], lineVertices[0], color);
	}

	void Renderer::SetLineWidth(float width)
	{
		data.LineWidth = width;
	}

	void Renderer::SetClearColor(glm::vec4 color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void Renderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void Renderer::SetMaxQuadsCount(uint32_t count)
	{
		data.MaxQuads = count;
		data.MaxVertices = data.MaxQuads * 4;
		data.MaxIndices = data.MaxQuads * 6;
	}

	uint32_t Renderer::GetDrawCallsCount()
	{
		return data.OpenGLDrawCalls;
	}

}
