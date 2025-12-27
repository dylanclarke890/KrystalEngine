#pragma once

#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageViewRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/SamplerRegistry.hpp"
#include "Krystal.Gfx/Registries/ITextureRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/Resources/Texture.hpp"
#include "Krystal.IO/ImageLoader.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/Pair.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class TextureRegistry final : public ITextureRegistry
  {
    NO_COPY_MOVE(TextureRegistry)

    using TextureManager = ResourceManager<Texture, TextureHandle>;
    using TextureCache = ResourceHandleCache<string, TextureHandle>;

    const IO::Path BaseDirectory {"/textures/"};

  private:
    IO::VirtualFileSystem &_vfs;
    ImageRegistry &_images;
    ImageViewRegistry &_imageViews;
    SamplerRegistry &_samplers;
    TextureManager _textures;
    TextureCache _cache;

  public:
    TextureRegistry(IO::VirtualFileSystem &vfs, ImageRegistry &images, ImageViewRegistry &imageViews,
                    SamplerRegistry &samplers) noexcept
        : _vfs(vfs), _images(images), _imageViews(imageViews), _samplers(samplers)
    {
    }

    ~TextureRegistry() override = default;

    void Startup() override
    {
    }

    void Shutdown() noexcept override
    {
      _cache.Clear();
    }

    KRYS_NODISCARD virtual TextureHandle Create(ImageViewHandle imageView,
                                            SamplerHandle sampler) noexcept override
    {
      static uint32 unnamedTextureCounter = 0u;
      Texture texture {imageView, sampler, TextureOwner::Other};
      return AddTexture(std::format("{}", unnamedTextureCounter++), std::move(texture));
    }

    KRYS_NODISCARD TextureHandle Load(const IO::Path &path, const TextureDesc &desc = {}) noexcept override
    {
      auto key = path.ToString();
      if (auto existing = _cache.Get(key); existing.IsValid())
      {
        return existing;
      }

      Unique<IO::IStreamReader> stream = _vfs.GetReader(BaseDirectory / path, IO::ReadFlags::None);
      IO::ImageLoader loader;
      auto imageResult = loader.Load(*stream, {.FlipVertically = true});
      assert(imageResult.has_value() && "Failed to load texture image.");
      auto &image = *imageResult;

      auto [internalFormat, dataFormat] = GetTextureFormat(image, desc);

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

      GLenum dataType = image.DataType == IO::ImageDataType::Float ? GL_FLOAT : GL_UNSIGNED_BYTE;
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

      auto &imgView = _imageViews.Get(imageViewHandle);
      glObjectLabel(GL_TEXTURE, img.Id(), -1, key.c_str());
      glObjectLabel(GL_TEXTURE, imgView.Id(), -1, (key + "-view").c_str());

      Texture texture {imageViewHandle, samplerHandle, TextureOwner::TextureRegistry};
      return AddTexture(key, std::move(texture));
    }

    KRYS_NODISCARD TextureHandle LoadCubemap(const IO::Path &left, const IO::Path &right, const IO::Path &top,
                                         const IO::Path &bottom, const IO::Path &front, const IO::Path &back,
                                         const TextureDesc &desc = {}) noexcept override
    {
      auto key = left.ToString() + "|" + right.ToString() + "|" + top.ToString() + "|" + bottom.ToString()
                 + "|" + front.ToString() + "|" + back.ToString();
      if (auto existing = _cache.Get(key); existing.IsValid())
      {
        return existing;
      }

      Array<IO::Path, 6> paths {right, left, top, bottom, front, back};
      List<IO::Image> images {};
      IO::ImageLoader loader;

      for (const auto &path : paths)
      {
        Unique<IO::IStreamReader> stream = _vfs.GetReader(BaseDirectory / path, IO::ReadFlags::None);
        assert(stream != nullptr && "Failed to open cubemap face image.");

        auto imageResult = loader.Load(*stream, {.FlipVertically = false});
        assert(imageResult.has_value() && "Failed to load cubemap face image.");

        auto &image = *imageResult;
        assert(image.Width == image.Height && "Cubemap face image must be square.");
        assert(image.Channels >= 3 && image.Channels <= 4 && "Cubemap face image must have 3 or 4 channels.");

        images.push_back(std::move(image));
      }

      auto [internalFormat, dataFormat] = GetTextureFormat(images[0], desc);

      uint32 maxMipLevels =
        (desc.GenerateMipmaps ? GetMipLevels(desc.MaxMipLevels, images[0].Width, images[0].Height) : 1u);

      ImageHandle imageHandle = _images.Create({
        .Type = ImageType::ImageCube,
        .Format = internalFormat,
        .Width = (uint32)images[0].Width,
        .Height = (uint32)images[0].Height,
        .Depth = 1,
        .MipLevels = maxMipLevels,
        .ArrayLayers = 6,
      });

      GLenum dataType = GL_UNSIGNED_BYTE;
      Image &img = _images.Get(imageHandle);
      for (int i = 0; i < 6; ++i)
      {
        img.UpdateData(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, images[i].Data, dataFormat, dataType);
      }

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

      Texture texture {imageViewHandle, samplerHandle, TextureOwner::TextureRegistry};
      return AddTexture(key, std::move(texture));
    }

    bool Unload(TextureHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid handle.");
      assert(_textures.TryGet(handle) != nullptr && "Texture not found in resource manager.");

      if (_cache.Remove(handle))
      {
        Texture &texture = _textures.Get(handle);
        ImageView &imageView = _imageViews.Get(texture.ImageView());

        _imageViews.Destroy(texture.ImageView());
        _samplers.Unload(texture.Sampler());
        if (texture.Owner() == TextureOwner::TextureRegistry)
        {
          _images.Destroy(imageView.Image());
        }

        return _textures.Remove(handle);
      }
      return false;
    }

    KRYS_NODISCARD Texture &Get(TextureHandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid texture handle.");
      return _textures.Get(handle);
    }

    KRYS_NODISCARD ImageView &GetView(TextureHandle handle) noexcept
    {
      assert(handle.IsValid() && "Invalid texture handle.");
      Texture &texture = Get(handle);

      assert(texture.ImageView().IsValid() && "Texture has an invalid image view.");
      return _imageViews.Get(texture.ImageView());
    }

    void Bind(TextureHandle handle, uint32 unit) noexcept
    {
      assert(handle.IsValid() && "Invalid texture handle.");

      Texture &texture = Get(handle);
      assert(texture.ImageView().IsValid() && "Texture has an invalid image view.");
      assert(texture.Sampler().IsValid() && "Texture has an invalid sampler.");

      ImageView &imageView = _imageViews.Get(texture.ImageView());
      imageView.Bind(unit);

      Sampler &sampler = _samplers.Get(texture.Sampler());
      sampler.Bind(unit);
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

    static Pair<PixelFormat, GLenum> GetTextureFormat(const IO::Image &image,
                                                      const TextureDesc &desc) noexcept
    {
      if (image.DataType == IO::ImageDataType::Float)
      {
        return GetFloatTextureFormat(image.Channels, true);
      }
      else if (desc.IsSRGB)
      {
        return GetSRGBTextureFormat(image.Channels);
      }
      else
      {
        return GetLinearTextureFormat(image.Channels);
      }
    }

    static Pair<PixelFormat, GLenum> GetLinearTextureFormat(int channels) noexcept
    {
      assert(channels >= 1 && channels <= 4 && "Invalid number of channels.");
      switch (channels)
      {
        case 1: return {PixelFormat::R8, GL_RED};
        case 2: return {PixelFormat::R8G8, GL_RG};
        case 3: return {PixelFormat::R8G8B8, GL_RGB};
        case 4: return {PixelFormat::R8G8B8A8, GL_RGBA};
      }
      return {PixelFormat::R8G8B8A8, GL_RGBA};
    }

    static Pair<PixelFormat, GLenum> GetSRGBTextureFormat(int channels) noexcept
    {
      assert((channels == 3 || channels == 4) && "Invalid number of channels.");
      switch (channels)
      {
        case 3: return {PixelFormat::SRGB8, GL_RGB};
        case 4: return {PixelFormat::SRGB8A8, GL_RGBA};
      }
      return {PixelFormat::SRGB8A8, GL_RGB};
    }

    static Pair<PixelFormat, GLenum> GetFloatTextureFormat(int channels, bool isHalfFloat) noexcept
    {
      assert(channels >= 1 && channels <= 4 && "Invalid number of channels.");
      switch (channels)
      {
        case 1: return {isHalfFloat ? PixelFormat::R16F : PixelFormat::R32F, GL_RED};
        case 2: return {isHalfFloat ? PixelFormat::R16G16F : PixelFormat::R32G32F, GL_RG};
        case 3: return {isHalfFloat ? PixelFormat::R16G16B16F : PixelFormat::R32G32B32F, GL_RGB};
        case 4: return {isHalfFloat ? PixelFormat::R16G16B16A16F : PixelFormat::R32G32B32A32F, GL_RGBA};
      }
      return {isHalfFloat ? PixelFormat::R16G16B16A16F : PixelFormat::R32G32B32A32F, GL_RGB};
    }
  };
}