//
// This file provides functionalities for managing OpenGL textures.
// From Hazel Engine Renderer OpenGL API:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Platform/OpenGL/OpenGLTexture.cpp
//
#include "ptpch.h"
#include "Proton/Graphics/Renderer/Texture.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace proton {

	Texture::Texture(uint32_t width, uint32_t height, bool fillDataWhitePixels)
		: m_Width(width), m_Height(height),
		m_InternalFormat(GL_RGBA8), m_DataFormat(GL_RGBA)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Object_ID);
		glTextureStorage2D(m_Object_ID, 1, m_InternalFormat, m_Width, m_Height);

		SetFilterMode(TextureFilterMode::Nearest);
		SetWrapMode(TextureWrapMode::Repeat);

		if (fillDataWhitePixels)
		{
			// Set texture data to full white texture
			size_t pixels = (size_t)width * (size_t)height;
			uint32_t* data = new uint32_t[pixels];
			memset(data, 0xffffffff, pixels * sizeof(uint32_t));
			SetData(data, pixels * sizeof(uint32_t));
			delete[] data;
		}
	}

	Texture::Texture(const std::string& path)
		: m_Path(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(("content/textures/" + path).c_str(), &width, &height, &channels, 0);
			
		if (data)
		{
			m_IsLoaded = true;
			m_Width = width;
			m_Height = height;

			if (channels == 4)
			{
				m_InternalFormat = GL_RGBA8;
				m_DataFormat = GL_RGBA;
			}
			else if (channels == 3)
			{
				m_InternalFormat = GL_RGB8;
				m_DataFormat = GL_RGB;
			}

			PT_CORE_ASSERT(m_InternalFormat & m_DataFormat && "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &m_Object_ID);
			glTextureStorage2D(m_Object_ID, 1, m_InternalFormat, m_Width, m_Height);

			SetFilterMode(TextureFilterMode::Nearest);
			SetWrapMode(TextureWrapMode::Repeat);

			glTextureSubImage2D(m_Object_ID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_Object_ID);
	}

	void Texture::SetData(void* data, size_t size)
	{
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		PT_CORE_ASSERT(size == m_Width * m_Height * bpp && "Data must be entire texture!");
		glTextureSubImage2D(m_Object_ID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void Texture::SetFilterMode(TextureFilterMode mode)
	{
		m_FilterMode = mode;
		if (mode == TextureFilterMode::Nearest)
		{
			glTextureParameteri(m_Object_ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(m_Object_ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else if (mode == TextureFilterMode::Linear)
		{
			glTextureParameteri(m_Object_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_Object_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}

	static GLenum ProtonWrapModeToOpenGL(TextureWrapMode mode)
	{
		switch (mode)
		{
			case TextureWrapMode::Repeat:        return GL_REPEAT;
			case TextureWrapMode::ClampToBorder: return GL_CLAMP_TO_BORDER;
			case TextureWrapMode::ClampToEdge:   return GL_CLAMP_TO_EDGE;
		}
		return GL_REPEAT;
	}

	void Texture::SetWrapMode(TextureWrapMode mode)
	{
		SetWrapMode(mode, mode);
	}

	void Texture::SetWrapMode(TextureWrapMode xMode, TextureWrapMode yMode)
	{
		m_WrapModeX = xMode; m_WrapModeY = yMode;
		GLenum sWrap = ProtonWrapModeToOpenGL(xMode);
		GLenum tWrap = ProtonWrapModeToOpenGL(yMode);
		glTextureParameteri(m_Object_ID, GL_TEXTURE_WRAP_S, sWrap);
		glTextureParameteri(m_Object_ID, GL_TEXTURE_WRAP_T, tWrap);
	}

	void Texture::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_Object_ID);
	}
}
