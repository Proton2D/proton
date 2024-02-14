//
// This file provides functionalities for managing OpenGL textures.
// From Hazel Engine Renderer OpenGL API:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Platform/OpenGL/OpenGLTexture.h
//
#pragma once

#include "Proton/Core/Base.h"

// Forward declaration
typedef unsigned int GLenum;

namespace proton {

	enum class TextureFilterMode
	{
		Nearest, Linear
	};

	enum class TextureWrapMode
	{
		Repeat, ClampToBorder, ClampToEdge
	};

	class Texture
	{
	public:
		Texture(uint32_t width, uint32_t height, bool fillDataWhitePixels = false);
		Texture(const std::string& path);
		virtual ~Texture();

		uint32_t GetOpenGL_ID() const { return m_Object_ID; }

		uint32_t GetWidth() const { return m_Width;  }
		uint32_t GetHeight() const { return m_Height; }
		const std::string& GetPath() const { return m_Path; }
		
		void Bind(uint32_t slot = 0) const;
		void SetData(void* data, size_t size);
		bool IsLoaded() const { return m_IsLoaded; }

		TextureFilterMode GetFilterMode() const { return m_FilterMode; }
		std::pair< TextureWrapMode, TextureWrapMode> GetWrapMode() const { return { m_WrapModeX, m_WrapModeY }; }

		void SetFilterMode(TextureFilterMode mode);
		void SetWrapMode(TextureWrapMode mode);
		void SetWrapMode(TextureWrapMode x_mode, TextureWrapMode y_mode);

		bool operator==(const Texture& other) const
		{
			return m_Object_ID == other.m_Object_ID;
		}

	private:
		bool m_IsLoaded = false;
		std::string m_Path;
		uint32_t m_Width = 0, m_Height = 0;
		uint32_t m_Object_ID = 0;
		GLenum m_InternalFormat = 0;
		GLenum m_DataFormat = 0;
		TextureFilterMode m_FilterMode = TextureFilterMode::Linear;
		TextureWrapMode m_WrapModeX = TextureWrapMode::Repeat;
		TextureWrapMode m_WrapModeY = TextureWrapMode::Repeat;

		friend class SceneSerializer;
	};

}
