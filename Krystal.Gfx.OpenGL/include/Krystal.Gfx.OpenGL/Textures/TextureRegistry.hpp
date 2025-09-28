#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Images/ImageRegistry.hpp"
#include "Krystal.Gfx.OpenGL/ImageViews/ImageViewRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Samplers/SamplerRegistry.hpp"
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
    ImageRegistry &_images;
    ImageViewRegistry &_imageViews;
    SamplerRegistry &_samplers;
    TextureManager _textures;
    TextureCache _cache;

  public:
    TextureRegistry(ImageRegistry &images, ImageViewRegistry &imageViews, SamplerRegistry &samplers) noexcept
        : _images(images), _imageViews(imageViews), _samplers(samplers)
    {
    }

    ~TextureRegistry() override = default;

    void Startup() noexcept override
    {
    }

    void Shutdown() noexcept override
    {
      _cache.Clear();
    }

    NO_DISCARD TextureHandle Load(const IO::Path &path, const TextureDesc &desc = {}) noexcept override
    {
      auto key = path.ToString();
      if (auto existing = _cache.Get(key); existing.IsValid())
      {
        return existing;
      }

      auto imageResult = IO::LoadImage(path, {.FlipVertically = true});
      assert(imageResult.has_value() && "Failed to load texture image.");
      auto &image = *imageResult;

      auto [internalFormat, dataFormat] = image.IsHDR   ? GetFloatTextureFormat(image.Channels, true)
                                          : desc.IsSRGB ? GetSRGBTextureFormat(image.Channels)
                                                        : GetLinearTextureFormat(image.Channels);

      uint32 maxMipLevels =
        (desc.GenerateMipmaps ? GetMipLevels(desc.MaxMipLevels, image.Width, image.Height) : 1u);

      ImageHandle imageHandle = _images.Create({
        .Type = ImageType::Image2D,
        .Format = internalFormat,
        .Width = (uint32)image.Width,
        .Height = (uint32)image.Height,
        .Depth = 1,
        .MipLevels = maxMipLevels,
        .ArrayLayers = 1,
      });

      GLenum dataType = image.IsHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
      Image &img = _images.Get(imageHandle);
      img.UpdateData(image.Data, dataFormat, dataType);

      if (desc.GenerateMipmaps)
      {
        img.GenerateMipmaps();
      }

      ImageViewHandle imageViewHandle =
        _imageViews.Create({.Image = imageHandle,
                            .Target = ImageType::Image2D,
                            .Format = internalFormat,
                            .SubResourceRange = {.BaseMipLevel = 0, .MipLevelCount = maxMipLevels}});

      SamplerDesc samplerDesc = {.MinFilter =
                                   desc.GenerateMipmaps ? FilterMode::LinearMipmapLinear : FilterMode::Linear,
                                 .MagFilter = FilterMode::Linear,
                                 .WrapS = WrapMode::ClampToEdge,
                                 .WrapT = WrapMode::ClampToEdge,
                                 .WrapR = WrapMode::ClampToEdge,
                                 .AnisotropicLevel = 1.f};
      SamplerHandle samplerHandle = _samplers.Create(samplerDesc);

      Texture texture {imageViewHandle, samplerHandle};
      return AddTexture(key, std::move(texture));
    }

    NO_DISCARD TextureHandle LoadCubemap(const IO::Path &left, const IO::Path &right, const IO::Path &top,
                                         const IO::Path &bottom, const IO::Path &front, const IO::Path &back,
                                         const TextureDesc &desc = {}) noexcept override
    {
      auto key = left.ToString() + "|" + right.ToString() + "|" + top.ToString() + "|" + bottom.ToString()
                 + "|" + front.ToString() + "|" + back.ToString();
      if (auto existing = _cache.Get(key); existing.IsValid())
      {
        return existing;
      }

      auto cubeMapResult = IO::LoadCubeMap(left, right, top, bottom, front, back, {.FlipVertically = false});
      assert(cubeMapResult.has_value() && "Failed to load cubemap image.");
      auto &cubeMap = *cubeMapResult;

      auto [internalFormat, dataFormat] =
        desc.IsSRGB ? GetSRGBTextureFormat(cubeMap.Channels) : GetLinearTextureFormat(cubeMap.Channels);

      uint32 maxMipLevels =
        (desc.GenerateMipmaps ? GetMipLevels(desc.MaxMipLevels, cubeMap.Width, cubeMap.Height) : 1u);

      ImageHandle imageHandle = _images.Create({
        .Type = ImageType::ImageCube,
        .Format = internalFormat,
        .Width = (uint32)cubeMap.Width,
        .Height = (uint32)cubeMap.Height,
        .Depth = 1,
        .MipLevels = maxMipLevels,
        .ArrayLayers = 6,
      });

      GLenum dataType = GL_UNSIGNED_BYTE;
      Image &img = _images.Get(imageHandle);
      img.UpdateData(GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubeMap.Right, dataFormat, dataType);
      img.UpdateData(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, cubeMap.Left, dataFormat, dataType);
      img.UpdateData(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, cubeMap.Top, dataFormat, dataType);
      img.UpdateData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, cubeMap.Bottom, dataFormat, dataType);
      img.UpdateData(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, cubeMap.Front, dataFormat, dataType);
      img.UpdateData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, cubeMap.Back, dataFormat, dataType);

      if (desc.GenerateMipmaps)
      {
        img.GenerateMipmaps();
      }

      ImageViewHandle imageViewHandle =
        _imageViews.Create({.Image = imageHandle,
                            .Target = ImageType::ImageCube,
                            .Format = internalFormat,
                            .SubResourceRange = {.BaseMipLevel = 0, .MipLevelCount = maxMipLevels}});

      FilterMode filterMode = desc.GenerateMipmaps ? FilterMode::LinearMipmapLinear : FilterMode::Linear;
      SamplerDesc samplerDesc = {.MinFilter = filterMode,
                                 .MagFilter = filterMode,
                                 .WrapS = WrapMode::ClampToEdge,
                                 .WrapT = WrapMode::ClampToEdge,
                                 .WrapR = WrapMode::ClampToEdge,
                                 .AnisotropicLevel = 1.f};
      SamplerHandle samplerHandle = _samplers.Create(samplerDesc);

      Texture texture {imageViewHandle, samplerHandle};
      return AddTexture(key, std::move(texture));
    }

    void Unload(TextureHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid handle.");
      assert(_textures.TryGet(handle) != nullptr && "Texture not found in resource manager.");

      if (_cache.Remove(handle))
      {
        Texture &texture = _textures.Get(handle);
        ImageView &imageView = _imageViews.Get(texture.ImageView());

        _imageViews.Unload(texture.ImageView());
        _images.Unload(imageView.Image());
        _samplers.Unload(texture.Sampler());

        _textures.Remove(handle);
      }
    }

    Texture &Get(TextureHandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid texture handle.");
      return _textures.Get(handle);
    }

    ImageView &GetView(TextureHandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid texture handle.");
      Texture &texture = Get(handle);

      assert(texture.ImageView().IsValid() && "Texture has an invalid image view.");
      return _imageViews.Get(texture.ImageView());
    }

  private:
    TextureHandle AddTexture(const string &key, Texture &&texture) noexcept
    {
      auto handle = _textures.Add(std::move(texture));
      _cache.Add(key, handle);
      return handle;
    }

    static uint32 GetMipLevels(uint32 mipLevels, GLsizei width, GLsizei height) noexcept
    {
      if (mipLevels != 0u)
      {
        return mipLevels;
      }
      return static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1u;
    }

    static Pair<ImageFormat, GLenum> GetLinearTextureFormat(int channels) noexcept
    {
      assert(channels >= 1 && channels <= 4 && "Invalid number of channels.");
      switch (channels)
      {
        case 1: return {ImageFormat::R8, GL_RED};
        case 2: return {ImageFormat::R8G8, GL_RG};
        case 3: return {ImageFormat::R8G8B8, GL_RGB};
        case 4: return {ImageFormat::R8G8B8A8, GL_RGBA};
      }
      return {ImageFormat::R8G8B8A8, GL_RGBA};
    }

    static Pair<ImageFormat, GLenum> GetSRGBTextureFormat(int channels) noexcept
    {
      assert((channels == 3 || channels == 4) && "Invalid number of channels.");
      switch (channels)
      {
        case 3: return {ImageFormat::SRGB8, GL_RGB};
        case 4: return {ImageFormat::SRGB8A8, GL_RGBA};
      }
      return {ImageFormat::SRGB8A8, GL_RGB};
    }

    static Pair<ImageFormat, GLenum> GetFloatTextureFormat(int channels, bool isHalfFloat) noexcept
    {
      assert(channels >= 1 && channels <= 4 && "Invalid number of channels.");
      switch (channels)
      {
        case 1: return {isHalfFloat ? ImageFormat::R16F : ImageFormat::R32F, GL_RED};
        case 2: return {isHalfFloat ? ImageFormat::R16G16F : ImageFormat::R32G32F, GL_RG};
        case 3: return {isHalfFloat ? ImageFormat::R16G16B16F : ImageFormat::R32G32B32F, GL_RGB};
        case 4: return {isHalfFloat ? ImageFormat::R16G16B16A16F : ImageFormat::R32G32B32A32F, GL_RGBA};
      }
      return {isHalfFloat ? ImageFormat::R16G16B16A16F : ImageFormat::R32G32B32A32F, GL_RGB};
    }
  };
}