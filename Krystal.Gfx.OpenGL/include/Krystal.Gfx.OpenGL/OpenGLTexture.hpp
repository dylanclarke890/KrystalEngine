#pragma once

#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.IO/Image.hpp"

#include <cassert>

namespace Krys::Gfx::OpenGL
{
  template <GLenum TextureType>
  requires(TextureType == GL_TEXTURE_2D || TextureType == GL_TEXTURE_CUBE_MAP)
  class OpenGLTexture
  {
  public:
    OpenGLTexture(const IO::Path &filepath,
                  const IO::ImageLoadSettings &settings = {.FlipVertically = true,
                                                           .DesiredComponents = 0}) noexcept
        : _handle(0)
    {
      glGenTextures(1, &_handle);
      glBindTexture(TextureType, _handle);

      if constexpr (TextureType == GL_TEXTURE_2D)
      {
        auto imageResult = IO::LoadImage(filepath, settings);
        assert(imageResult.has_value() && "Failed to load texture image.");

        auto &image = *imageResult;
        assert((image.Channels == 3 || image.Channels == 4) && "Texture image data must be 3 or 4 channels.");

        GLint internalFormat = GL_RGB;
        GLenum format = GL_RGB;
        if (image.Channels == 4)
        {
          internalFormat = GL_RGBA;
          format = GL_RGBA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.Width, image.Height, 0, format, GL_UNSIGNED_BYTE,
                     image.Data.data());

        glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
      else if constexpr (TextureType == GL_TEXTURE_CUBE_MAP)
      {
        auto cubeMapResult = IO::LoadCubeMap(filepath, settings);
        assert(cubeMapResult.has_value() && "Failed to load cubemap image.");

        auto &cubeMap = *cubeMapResult;
        assert((cubeMap.Channels == 3 || cubeMap.Channels == 4)
               && "Cubemap image data must be 3 or 4 channels.");

        GLint internalFormat = GL_RGB;
        GLenum format = GL_RGB;
        if (cubeMap.Channels == 4)
        {
          internalFormat = GL_RGBA;
          format = GL_RGBA;
        }

#define UploadFace(face, faceData)                                                                           \
  glTexImage2D(face, 0, internalFormat, cubeMap.Width, cubeMap.Height, 0, format, GL_UNSIGNED_BYTE,          \
               faceData.data());

        UploadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, cubeMap.Left);
        UploadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubeMap.Right);
        UploadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, cubeMap.Top);
        UploadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, cubeMap.Bottom);
        UploadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, cubeMap.Front);
        UploadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, cubeMap.Back);

#undef UploadFace

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      }
    }

    ~OpenGLTexture() noexcept
    {
      glDeleteTextures(1, &_handle);
    }

    void Bind(GLuint unit = 0u) const noexcept
    {
      glActiveTexture(GL_TEXTURE0 + unit);
      glBindTexture(TextureType, _handle);
    }

  private:
    GLuint _handle;
  };

  using OpenGLTexture2D = OpenGLTexture<GL_TEXTURE_2D>;
  using OpenGLCubeMap = OpenGLTexture<GL_TEXTURE_CUBE_MAP>;
}