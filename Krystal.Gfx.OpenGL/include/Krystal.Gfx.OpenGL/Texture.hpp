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
    GLuint _handle;

  public:
    Texture(const IO::Path &filepath,
            const IO::ImageLoadSettings &settings = {.FlipVertically = true, .DesiredComponents = 0}) noexcept
    requires(TextureType == GL_TEXTURE_2D)
        : _handle(0u)
    {
      Load(filepath, settings, IsSRGBTexture(false), IsHDRTexture(false));
    }

    Texture(const IO::Path &filepath, IsSRGBTexture isSRGBTexture,
            const IO::ImageLoadSettings &settings = {.FlipVertically = true, .DesiredComponents = 0}) noexcept
    requires(TextureType == GL_TEXTURE_2D)
        : _handle(0u)
    {
      Load(filepath, settings, isSRGBTexture, IsHDRTexture(false));
    }

    Texture(const IO::Path &filepath, IsHDRTexture isHDRTexture,
            const IO::ImageLoadSettings &settings = {.FlipVertically = true, .DesiredComponents = 0}) noexcept
    requires(TextureType == GL_TEXTURE_2D)
        : _handle(0u)
    {
      Load(filepath, settings, IsSRGBTexture(false), isHDRTexture);
    }

    Texture(const IO::Path &left, const IO::Path &right, const IO::Path &top, const IO::Path &bottom,
            const IO::Path &front, const IO::Path &back, IsSRGBTexture isSRGBTexture = IsSRGBTexture(false),
            const IO::ImageLoadSettings &settings = {.FlipVertically = false,
                                                     .DesiredComponents = 0}) noexcept
    requires(TextureType == GL_TEXTURE_CUBE_MAP)
        : _handle(0u)
    {
      Load(left, right, top, bottom, front, back, settings, isSRGBTexture);
    }

    ~Texture() noexcept
    {
      glDeleteTextures(1, &_handle);
    }

    void Bind(GLuint unit = 0u) const noexcept
    {
      glBindTextureUnit(unit, _handle);
    }

    void SetParameter(GLenum pname, GLint param) noexcept
    {
      glTextureParameteri(_handle, pname, param);
    }

  private:
    static Pair<GLenum, GLenum> GetLinearTextureFormat(int channels) noexcept
    {
      assert(channels >= 1 && channels <= 4 && "Invalid number of channels.");
      switch (channels)
      {
        case 1: return {GL_R8, GL_RED};
        case 2: return {GL_RG8, GL_RG};
        case 3: return {GL_RGB8, GL_RGB};
        case 4: return {GL_RGBA8, GL_RGBA};
      }
      return {GL_RGB8, GL_RGB};
    }

    static Pair<GLenum, GLenum> GetSRGBTextureFormat(int channels) noexcept
    {
      assert(channels > 2 && channels <= 4 && "Invalid number of channels.");
      switch (channels)
      {
        case 3: return {GL_SRGB8, GL_RGB};
        case 4: return {GL_SRGB8_ALPHA8, GL_RGBA};
      }
      return {GL_SRGB8, GL_RGB};
    }

    static Pair<GLenum, GLenum> GetFloatTextureFormat(int channels, bool isHalfFloat) noexcept
    {
      assert(channels >= 1 && channels <= 4 && "Invalid number of channels.");
      switch (channels)
      {
        case 1: return {isHalfFloat ? GL_R16F : GL_R32F, GL_RED};
        case 2: return {isHalfFloat ? GL_RG16F : GL_RG32F, GL_RG};
        case 3: return {isHalfFloat ? GL_RGB16F : GL_RGB32F, GL_RGB};
        case 4: return {isHalfFloat ? GL_RGBA16F : GL_RGBA32F, GL_RGBA};
      }
      return {isHalfFloat ? GL_RGB16F : GL_RGB32F, GL_RGB};
    }

    void Load(const IO::Path &filepath, const IO::ImageLoadSettings &settings, IsSRGBTexture isSRGBTexture,
              IsHDRTexture isHDRTexture) noexcept
    {
      assert(isSRGBTexture == false || isHDRTexture == false && "A texture cannot be both sRGB and HDR.");
      glCreateTextures(TextureType, 1, &_handle);

      glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (isHDRTexture)
      {
        auto imageResult = IO::LoadHDRImage(filepath, settings);
        assert(imageResult.has_value() && "Failed to load texture image.");
        auto &image = *imageResult;

        auto [internalFormat, format] = GetFloatTextureFormat(image.Channels, false);
        glTextureStorage2D(_handle, 1, internalFormat, image.Width, image.Height);
        glTextureSubImage2D(_handle, 0, 0, 0, image.Width, image.Height, format, GL_FLOAT, image.Data.data());
      }
      else
      {
        auto imageResult = IO::LoadImage(filepath, settings);
        assert(imageResult.has_value() && "Failed to load texture image.");
        auto &image = *imageResult;

        auto [internalFormat, format] =
          isSRGBTexture ? GetSRGBTextureFormat(image.Channels) : GetLinearTextureFormat(image.Channels);

        glTextureStorage2D(_handle, 1, internalFormat, image.Width, image.Height);
        glTextureSubImage2D(_handle, 0, 0, 0, image.Width, image.Height, format, GL_UNSIGNED_BYTE,
                            image.Data.data());
      }
    }

    void Load(const IO::Path &left, const IO::Path &right, const IO::Path &top, const IO::Path &bottom,
              const IO::Path &front, const IO::Path &back, const IO::ImageLoadSettings &settings,
              IsSRGBTexture isSRGBTexture) noexcept
    {
      glCreateTextures(TextureType, 1, &_handle);

      glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTextureParameteri(_handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      auto cubeMapResult = IO::LoadCubeMap(left, right, top, bottom, front, back, settings);
      assert(cubeMapResult.has_value() && "Failed to load cubemap image.");

      auto &cubeMap = *cubeMapResult;
      auto [internalFormat, format] =
        isSRGBTexture ? GetSRGBTextureFormat(cubeMap.Channels) : GetLinearTextureFormat(cubeMap.Channels);

      glTextureStorage2D(_handle, 1, internalFormat, cubeMap.Width, cubeMap.Height);

#define UploadFace(face, faceData)                                                                           \
  glTextureSubImage3D(_handle, 0, 0, 0, face - GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubeMap.Width,                \
                      cubeMap.Height, 1, format, GL_UNSIGNED_BYTE, faceData.data());

      UploadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, cubeMap.Left);
      UploadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubeMap.Right);
      UploadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, cubeMap.Top);
      UploadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, cubeMap.Bottom);
      UploadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, cubeMap.Front);
      UploadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, cubeMap.Back);

#undef UploadFace
    }
  };

  using Texture2D = Texture<GL_TEXTURE_2D>;
  using CubeMap = Texture<GL_TEXTURE_CUBE_MAP>;
}