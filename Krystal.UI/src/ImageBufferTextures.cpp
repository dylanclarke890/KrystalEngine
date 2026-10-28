#include "Krystal.UI/ImageBufferTextures.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ImageBufferDisplayListBackend.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Colour/ColourUtilities.hpp"
#include "Krystal.UI/Compositing/EngineTextureLayer.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "Krystal.UI/ImageBufferRender2DBackend.hpp"
#include "Krystal.UI/TextureImage.hpp"

namespace krys::ui
{
  ImageBufferTextures::ImageBufferTextures(gfx::Device &device, FontFaceCache &fonts) noexcept
      : _device(device), _fonts(fonts)
  {
  }

  ImageBufferTextures::~ImageBufferTextures() noexcept
  {
    _filters = nullptr;
    _renderers.clear();
    for (const auto &pooled : _pool)
    {
      _device.Destroy(pooled.Texture);
    }
    for (const auto &[identifier, uploaded] : _uploaded)
    {
      _device.Destroy(uploaded.Texture);
    }
  }

  gfx::TextureHandle ImageBufferTextures::Render(boo::ImageBuffer &buffer,
                                                 gfx::CommandBuffer &commands) noexcept
  {
    // Krystal.UI makes every accelerated buffer. Painting it draws everything drawn so far.
    if (buffer.GetRenderingMode() == boo::RenderingMode::Accelerated)
    {
      auto *accelerated = static_cast<ImageBufferRender2DBackend *>(buffer.Backend());
      if (!accelerated)
      {
        return {};
      }
      accelerated->FlushContext();
      return Render(*accelerated, commands);
    }

    const auto identifier = buffer.GetRenderingResourceIdentifier().ToUInt64();
    if (const auto found = _rendered.find(identifier); found != _rendered.end())
    {
      return found->second;
    }

    // Every other image buffer records into a display list.
    auto *backend = static_cast<boo::ImageBufferDisplayListBackend *>(buffer.Backend());
    const auto size = buffer.BackendSize();
    if (!backend || size.IsEmpty())
    {
      return {};
    }
    const auto width = static_cast<uint32>(size.Width());
    const auto height = static_cast<uint32>(size.Height());
    const auto texture = Acquire(width, height);
    auto *renderer = RendererForDepth();
    if (!texture.IsValid() || !renderer)
    {
      return {};
    }

    const auto displayList = backend->CopyDisplayList();
    ++_depth;
    renderer->Begin(commands, {.Texture = texture,
                               .Width = width,
                               .Height = height,
                               .Format = gfx::TextureFormat::RGBA8Unorm,
                               .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
    // A luminance mask's drawing goes through a layer that turns its luminance into alpha.
    if (backend->IsLuminanceMask())
    {
      renderer->BeginLayer(1.f, BlendMode::Normal, gfx::LayerFilter2D::LuminanceToAlpha());
    }
    {
      GraphicsContextRender2D context(
        *renderer, _fonts, {0.f, 0.f, static_cast<float>(width), static_cast<float>(height)}, this);
      context.DrawDisplayList(*displayList);
    }
    if (backend->IsLuminanceMask())
    {
      renderer->EndLayer();
    }
    renderer->End();
    --_depth;

    _rendered[identifier] = texture;
    return texture;
  }

  gfx::TextureHandle ImageBufferTextures::Render(ImageBufferRender2DBackend &backend,
                                                 gfx::CommandBuffer &commands) noexcept
  {
    auto *renderer = RendererForDepth();
    if (!renderer)
    {
      return {};
    }
    ++_depth;
    const auto texture = backend.Render(*renderer, _fonts, *this, commands);
    --_depth;
    return texture;
  }

  gfx::TextureHandle ImageBufferTextures::Copy(gfx::TextureHandle texture, const gfx::Rect2D &source,
                                               uint32 width, uint32 height,
                                               gfx::CommandBuffer &commands) noexcept
  {
    const auto copy = Acquire(width, height);
    auto *renderer = RendererForDepth();
    if (!copy.IsValid() || !renderer)
    {
      return {};
    }
    renderer->Begin(commands, {.Texture = copy,
                               .Width = width,
                               .Height = height,
                               .Format = gfx::TextureFormat::RGBA8Unorm,
                               .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
    renderer->DrawImage(texture, source, {0.f, 0.f, static_cast<float>(width), static_cast<float>(height)});
    renderer->End();
    return copy;
  }

  gfx::TextureHandle ImageBufferTextures::Upload(boo::NativeImage &image) noexcept
  {
    // An engine texture or a snapshot is drawn as it is.
    if (const auto &platformImage = image.GetPlatformImage(); platformImage && platformImage->External())
    {
      // Krystal.UI makes every external image.
      return static_cast<TextureImage &>(*platformImage->External()).Handle();
    }

    const auto identifier = image.GetRenderingResourceIdentifier().ToUInt64();
    if (const auto found = _uploaded.find(identifier); found != _uploaded.end())
    {
      return found->second.Texture;
    }

    const auto &platformImage = image.GetPlatformImage();
    if (!platformImage || platformImage->Size().IsEmpty())
    {
      return {};
    }
    const auto width = static_cast<uint32>(platformImage->Size().Width());
    const auto height = static_cast<uint32>(platformImage->Size().Height());
    auto texture = _device.CreateTexture({.Format = gfx::TextureFormat::BGRA8Unorm,
                                          .Width = width,
                                          .Height = height,
                                          .Usage = gfx::TextureUsage::Sampled,
                                          .DebugName = u8"UI.NativeImage"});
    if (!texture)
    {
      return {};
    }

    // ARGB in a 32-bit value is BGRA in memory. The renderer blends premultiplied colours.
    auto pixels = platformImage->Pixels();
    List<uint32> premultiplied;
    if (!platformImage->PremultipliedAlpha())
    {
      premultiplied.reserve(pixels.size());
      for (const auto pixel : pixels)
      {
        premultiplied.push_back(
          PackedColour::ARGB {PremultipliedFlooring(AsSRGBA(PackedColour::ARGB {pixel}))}.Value);
      }
      pixels = premultiplied;
    }
    _device.WriteTexture(*texture, gfx::TextureRegion::Whole2D(width, height),
                         {reinterpret_cast<const byte *>(pixels.data()), pixels.size_bytes()});

    _uploaded[identifier] = {.Image = WeakPtr<boo::NativeImage>::WithRef(image), .Texture = *texture};
    return *texture;
  }

  gfx::FilterRenderer2D *ImageBufferTextures::Filters() noexcept
  {
    if (!_filters)
    {
      auto filters = gfx::FilterRenderer2D::Create(_device);
      if (!filters)
      {
        return nullptr;
      }
      _filters = krys::move(*filters);
    }
    return _filters.get();
  }

  void ImageBufferTextures::Release() noexcept
  {
    for (auto &pooled : _pool)
    {
      pooled.InUse = false;
    }
    _rendered.clear();

    for (auto it = _uploaded.begin(); it != _uploaded.end();)
    {
      if (!it->second.Image)
      {
        _device.Destroy(it->second.Texture);
        it = _uploaded.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  gfx::TextureHandle ImageBufferTextures::Acquire(uint32 width, uint32 height) noexcept
  {
    for (auto &pooled : _pool)
    {
      if (!pooled.InUse && pooled.Width == width && pooled.Height == height)
      {
        pooled.InUse = true;
        return pooled.Texture;
      }
    }
    auto texture =
      _device.CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                             .Width = width,
                             .Height = height,
                             .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                             .DebugName = u8"UI.ImageBuffer"});
    if (!texture)
    {
      return {};
    }
    _pool.push_back({.Texture = *texture, .Width = width, .Height = height, .InUse = true});
    return *texture;
  }

  gfx::Renderer2D *ImageBufferTextures::RendererForDepth() noexcept
  {
    while (_renderers.size() <= _depth)
    {
      auto renderer = gfx::Renderer2D::Create(_device);
      if (!renderer)
      {
        return nullptr;
      }
      _renderers.push_back(krys::move(*renderer));
    }
    return _renderers[_depth].get();
  }
}
