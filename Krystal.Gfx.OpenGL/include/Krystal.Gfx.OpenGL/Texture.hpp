#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.IO/Image.hpp"

#include <cassert>

namespace Krys::Gfx::OpenGL
{
  struct IsSRGBTexture : TypedBool<IsSRGBTexture>
  {
    explicit constexpr IsSRGBTexture(bool value) noexcept : TypedBool<IsSRGBTexture>(value)
    {
    }
  };

  struct IsHDRTexture : TypedBool<IsHDRTexture>
  {
    explicit constexpr IsHDRTexture(bool value) noexcept : TypedBool<IsHDRTexture>(value)
    {
    }
  };

  template <GLenum TextureType>
  requires(TextureType == GL_TEXTURE_2D || TextureType == GL_TEXTURE_CUBE_MAP)
  class Texture
  {
  public:
    Texture(const IO::Path &filepath,
            const IO::ImageLoadSettings &settings = {.FlipVertically = true, .DesiredComponents = 0}) noexcept
        : _handle(0)
    {
      static_assert(TextureType == GL_TEXTURE_2D, "This constructor is only for 2d textures.");

      glGenTextures(1, &_handle);
      glBindTexture(TextureType, _handle);

      auto imageResult = IO::LoadImage(filepath, settings);
      assert(imageResult.has_value() && "Failed to load texture image.");

      auto &image = *imageResult;
      assert((image.Channels == 1 || image.Channels == 3 || image.Channels == 4)
             && "Texture image data must be 1, 3 or 4 channels.");

      GLint internalFormat =  GL_RGB;
      GLenum format = GL_RGB;
      if (image.Channels == 1)
      {
        internalFormat = GL_RED;
        format = GL_RED;
      }
      else if (image.Channels == 4)
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

    Texture(const IO::Path &filepath, IsSRGBTexture isSRGBTexture,
            const IO::ImageLoadSettings &settings = {.FlipVertically = true, .DesiredComponents = 0}) noexcept
        : _handle(0)
    {
      static_assert(TextureType == GL_TEXTURE_2D, "This constructor is only for 2d textures.");

      glGenTextures(1, &_handle);
      glBindTexture(TextureType, _handle);

      auto imageResult = IO::LoadImage(filepath, settings);
      assert(imageResult.has_value() && "Failed to load texture image.");

      auto &image = *imageResult;
      assert((image.Channels == 1 || image.Channels == 3 || image.Channels == 4)
             && "Texture image data must be 1, 3 or 4 channels.");

      GLint internalFormat = isSRGBTexture ? GL_SRGB : GL_RGB;
      GLenum format = GL_RGB;
      if (image.Channels == 1)
      {
        internalFormat = GL_RED;
        format = GL_RED;
      }
      else if (image.Channels == 4)
      {
        internalFormat = isSRGBTexture ? GL_SRGB_ALPHA : GL_RGBA;
        format = GL_RGBA;
      }

      glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.Width, image.Height, 0, format, GL_UNSIGNED_BYTE,
                   image.Data.data());

      glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    Texture(const IO::Path &filepath, IsHDRTexture isHDRTexture,
            const IO::ImageLoadSettings &settings = {.FlipVertically = true, .DesiredComponents = 0}) noexcept
        : _handle(0)
    {
      static_assert(TextureType == GL_TEXTURE_2D, "This constructor is only for 2d textures.");
      assert(isHDRTexture == true && "This constructor is only for 2d HDR textures.");

      glGenTextures(1, &_handle);
      glBindTexture(TextureType, _handle);

      auto imageResult = IO::LoadHDRImage(filepath, settings);
      assert(imageResult.has_value() && "Failed to load texture image.");

      auto &image = *imageResult;
      assert((image.Channels == 3 || image.Channels == 4) && "Texture image data must be 3 or 4 channels.");

      GLint internalFormat = GL_RGB16F;
      GLenum format = GL_RGB;
      if (image.Channels == 4)
      {
        internalFormat = GL_RGBA16F;
        format = GL_RGBA;
      }

      glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.Width, image.Height, 0, format, GL_FLOAT,
                   image.Data.data());

      glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    Texture(const IO::Path &left, const IO::Path &right, const IO::Path &top, const IO::Path &bottom,
            const IO::Path &front, const IO::Path &back,
            const IO::ImageLoadSettings &settings = {.FlipVertically = false,
                                                     .DesiredComponents = 0}) noexcept
        : _handle(0)
    {
      static_assert(TextureType == GL_TEXTURE_CUBE_MAP, "This constructor is only for cubemaps.");

      glGenTextures(1, &_handle);
      glBindTexture(TextureType, _handle);

      auto cubeMapResult = IO::LoadCubeMap(left, right, top, bottom, front, back, settings);
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

      glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    ~Texture() noexcept
    {
      glDeleteTextures(1, &_handle);
    }

    void Bind(GLuint unit = 0u) const noexcept
    {
      glActiveTexture(GL_TEXTURE0 + unit);
      glBindTexture(TextureType, _handle);
    }

    void SetParameter(GLenum pname, GLint param) noexcept
    {
      glTextureParameteri(_handle, pname, param);
    }

  private:
    GLuint _handle;
  };

  using Texture2D = Texture<GL_TEXTURE_2D>;
  using CubeMap = Texture<GL_TEXTURE_CUBE_MAP>;
}