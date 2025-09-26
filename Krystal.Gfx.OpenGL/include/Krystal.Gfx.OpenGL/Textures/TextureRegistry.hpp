#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Textures/Texture.hpp"
#include "Krystal.Gfx/Registries/ITextureRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.IO/Image.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/Pair.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class TextureRegistry final : public ITextureRegistry
  {
    NO_COPY_MOVE(TextureRegistry)

    using TextureManager = ResourceManager<Texture, TextureHandle>;
    using TextureCache = ResourceHandleCache<string, TextureHandle>;

  private:
    TextureManager _textures;
    TextureCache _cache;

  public:
    TextureRegistry() = default;
    ~TextureRegistry() override = default;

    void Startup() noexcept override
    {
    }

    void Shutdown() noexcept override
    {
    }

    NO_DISCARD TextureHandle Load(const IO::Path &path, const TextureDesc &desc = {}) noexcept override
    {
      auto key = path.ToString();
      auto existing = _cache.Get(key);
      if (existing.IsValid())
      {
        return existing;
      }

      auto imageResult = IO::LoadImage(path, {.FlipVertically = true});
      assert(imageResult.has_value() && "Failed to load texture image.");
      auto &image = *imageResult;

      auto [internalFormat, dataFormat] = image.IsHDR   ? GetFloatTextureFormat(image.Channels, true)
                                          : desc.IsSRGB ? GetSRGBTextureFormat(image.Channels)
                                                        : GetLinearTextureFormat(image.Channels);
      auto dataType = image.IsHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;

      uint32 maxMipLevels =
        (desc.GenerateMipmaps ? GetMipLevels(desc.MaxMipLevels, image.Width, image.Height) : 1u);

      Texture texture {internalFormat, image.Width, image.Height, maxMipLevels};
      texture.SetData(image.Data, dataFormat, dataType);

      if (desc.GenerateMipmaps && maxMipLevels > 1u)
      {
        texture.GenerateMipmaps();
        texture.SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      }

      auto handle = _textures.Add(std::move(texture));
      _cache.Add(key, handle);

      assert(handle.IsValid() && "Failed to load texture.");
      return handle;
    }

    NO_DISCARD TextureHandle LoadCubemap(const IO::Path &left, const IO::Path &right, const IO::Path &top,
                                         const IO::Path &bottom, const IO::Path &front, const IO::Path &back,
                                         const TextureDesc &desc = {}) noexcept override
    {
      auto key = left.ToString() + "|" + right.ToString() + "|" + top.ToString() + "|" + bottom.ToString()
                 + "|" + front.ToString() + "|" + back.ToString();
      auto existing = _cache.Get(key);
      if (existing.IsValid())
      {
        return existing;
      }

      auto cubeMapResult = IO::LoadCubeMap(left, right, top, bottom, front, back, {.FlipVertically = false});
      assert(cubeMapResult.has_value() && "Failed to load cubemap image.");
      auto &cubeMap = *cubeMapResult;

      auto [internalFormat, dataFormat] =
        desc.IsSRGB ? GetSRGBTextureFormat(cubeMap.Channels) : GetLinearTextureFormat(cubeMap.Channels);
      auto dataType = GL_UNSIGNED_BYTE;

      uint32 maxMipLevels =
        (desc.GenerateMipmaps ? GetMipLevels(desc.MaxMipLevels, cubeMap.Width, cubeMap.Height) : 1u);

      Texture texture {internalFormat, cubeMap.Width, cubeMap.Height, maxMipLevels, IsCubemap {}};
      texture.SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubeMap.Right, dataFormat, dataType);
      texture.SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, cubeMap.Left, dataFormat, dataType);
      texture.SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, cubeMap.Top, dataFormat, dataType);
      texture.SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, cubeMap.Bottom, dataFormat, dataType);
      texture.SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, cubeMap.Front, dataFormat, dataType);
      texture.SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, cubeMap.Back, dataFormat, dataType);

      if (desc.GenerateMipmaps && maxMipLevels > 1u)
      {
        texture.GenerateMipmaps();
        texture.SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      }

      auto handle = _textures.Add(std::move(texture));
      _cache.Add(key, handle);

      return handle;
    }

    void Unload(TextureHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid handle.");
      assert(_textures.TryGet(handle) != nullptr && "Texture not found in resource manager.");

      if (_cache.Remove(handle))
      {
        _textures.Remove(handle);
      }
    }

    Texture &Get(TextureHandle handle) noexcept
    {
      return _textures.Get(handle);
    }

  private:
    static uint32 GetMipLevels(uint32 mipLevels, GLsizei width, GLsizei height) noexcept
    {
      if (mipLevels != 0u)
      {
        return mipLevels;
      }

      return static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1u;
    }

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
      assert((channels == 3 || channels == 4) && "Invalid number of channels.");
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
  };
}