#include "Krystal.UI/ImageBufferRender2DBackend.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Graphics/PixelBuffer.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Gfx/CommandBuffer.hpp"
#include "Krystal.Gfx/Device.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "Krystal.UI/ImageBufferTextures.hpp"

namespace krys::ui
{
  namespace dl = boo::display_list;

  ImageBufferRender2DLayer::ImageBufferRender2DLayer(ImageBufferRender2DBackend &backend) noexcept
      : _backend(CreateWeakPtr(&backend))
  {
  }

  void ImageBufferRender2DLayer::PaintToTextureMapper(TextureMapper &textureMapper,
                                                      const boo::FloatRect &targetRect,
                                                      const boo::TransformationMatrix &modelViewMatrix,
                                                      float opacity) noexcept
  {
    auto *backend = _backend.get();
    if (!backend)
    {
      return;
    }
    if (const auto texture = textureMapper.RenderImageBuffer(*backend); texture.IsValid())
    {
      textureMapper.DrawTexture(texture, targetRect, modelViewMatrix, opacity,
                                AlphaPremultiplication::Premultiplied);
    }
  }

  UniquePtr<ImageBufferRender2DBackend>
    ImageBufferRender2DBackend::Create(const Parameters &parameters, AcceleratedImageBuffers &host) noexcept
  {
    return CreateUnique<ImageBufferRender2DBackend>(parameters, host);
  }

  ImageBufferRender2DBackend::ImageBufferRender2DBackend(const Parameters &parameters,
                                                         AcceleratedImageBuffers &host) noexcept
      : ImageBufferBackend(parameters), _drawingContext(boo::FloatSize(parameters.BackendSize), *this),
        _host(CreateWeakPtr(&host)), _delegate(ImageBufferRender2DDisplayDelegate::Create(*this))
  {
    // The drawing starts in the recorder's initial state.
    const auto &initial = _drawingContext.StateStack().front();
    _flushedState.push_back({.State = initial.State, .Ctm = initial.Ctm});
    host.Add(*this);
  }

  ImageBufferRender2DBackend::~ImageBufferRender2DBackend() noexcept
  {
    ReleaseTexture();
  }

  RefPtr<boo::ImageBuffer>
    ImageBufferRender2DBackend::Recorder::CreateImageBuffer(const boo::FloatSize &size,
                                                            float resolutionScale) const noexcept
  {
    if (auto *host = _backend._host.get())
    {
      return host->Create(size, boo::RenderingPurpose::Unspecified, resolutionScale);
    }
    return RecorderImpl::CreateImageBuffer(size, resolutionScale);
  }

  void ImageBufferRender2DBackend::Recorder::DrawImageBuffer(boo::ImageBuffer &buffer,
                                                             const boo::FloatRect &destRect,
                                                             const boo::FloatRect &srcRect,
                                                             boo::ImagePaintingOptions options) noexcept
  {
    // An accelerated buffer is drawn as it is now, not as it is when this is rendered: its later drawing,
    // and drawing it into itself, mustn't show. Without a device yet, it's drawn as it is when rendered.
    auto *host = _backend._host.get();
    if (buffer.GetRenderingMode() == boo::RenderingMode::Accelerated && host && host->Device())
    {
      if (auto image = buffer.CopyNativeImage())
      {
        DrawNativeImage(*image, destRect, srcRect, options);
      }
      return;
    }
    RecorderImpl::DrawImageBuffer(buffer, destRect, srcRect, options);
  }

  boo::GraphicsContext &ImageBufferRender2DBackend::Context() noexcept
  {
    return _drawingContext;
  }

  void ImageBufferRender2DBackend::FlushContext() noexcept
  {
    Flush();
  }

  void ImageBufferRender2DBackend::PrepareForDisplay() noexcept
  {
    Flush();
  }

  RefPtr<boo::GraphicsLayerContentsDisplayDelegate>
    ImageBufferRender2DBackend::LayerContentsDisplayDelegate() const noexcept
  {
    return _delegate;
  }

  RefPtr<boo::NativeImage> ImageBufferRender2DBackend::CopyNativeImage() noexcept
  {
    Flush();
    auto *host = _host.get();
    return host ? host->Snapshot(*this) : nullptr;
  }

  RefPtr<boo::NativeImage> ImageBufferRender2DBackend::CreateNativeImageReference() noexcept
  {
    // The texture changes as the buffer's drawing is rendered, so even a reference is a copy.
    return CopyNativeImage();
  }

  void ImageBufferRender2DBackend::GetPixelBuffer(const boo::IntRect &sourceRect,
                                                  boo::PixelBuffer &destination) noexcept
  {
    Flush();
    auto *host = _host.get();
    const auto texture = host ? host->RenderNow(*this) : gfx::TextureHandle {};
    if (!texture.IsValid())
    {
      destination.ZeroFill();
      return;
    }

    const auto width = static_cast<uint32>(Size().Width());
    const auto height = static_cast<uint32>(Size().Height());
    List<uint8> pixels(static_cast<size_t>(BytesPerRow()) * height);
    host->Device()->ReadTexture(texture, gfx::TextureRegion::Whole2D(width, height),
                                {reinterpret_cast<byte *>(pixels.data()), pixels.size()});
    ImageBufferBackend::GetPixelBuffer(sourceRect, Span<const uint8>(pixels.data(), pixels.size()),
                                       destination);
  }

  gfx::TextureHandle ImageBufferRender2DBackend::Render(gfx::Renderer2D &renderer, FontFaceCache &fonts,
                                                        ImageBufferTextures &imageBuffers,
                                                        gfx::CommandBuffer &commands) noexcept
  {
    // The view whose page made the buffer is gone, and its texture with it.
    if (!_host)
    {
      return {};
    }

    const auto width = static_cast<uint32>(Size().Width());
    const auto height = static_cast<uint32>(Size().Height());
    const bool isNew = !_texture.IsValid();
    if (isNew)
    {
      auto &device = renderer.GetDevice();
      auto texture =
        device.CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                              .Width = width,
                              .Height = height,
                              .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                              .DebugName = u8"UI.AcceleratedImageBuffer"});
      if (!texture)
      {
        return {};
      }
      _device = &device;
      _texture = *texture;
    }
    if (_pending.empty() && !isNew)
    {
      return _texture;
    }

    // The texture keeps what was drawn before; a new one starts transparent black.
    renderer.Begin(commands,
                   {.Texture = _texture,
                    .Width = width,
                    .Height = height,
                    .Format = gfx::TextureFormat::RGBA8Unorm,
                    .ClearColour = isNew ? Maybe<Array<float, 4>>(Array<float, 4> {0.f, 0.f, 0.f, 0.f})
                                         : Maybe<Array<float, 4>> {}});
    for (const auto &displayList : _pending)
    {
      GraphicsContextRender2D context(
        renderer, fonts, {0.f, 0.f, static_cast<float>(width), static_cast<float>(height)}, &imageBuffers);
      context.DrawDisplayList(*displayList);
    }
    renderer.End();
    _pending.clear();
    return _texture;
  }

  void ImageBufferRender2DBackend::ReleaseTexture() noexcept
  {
    if (_device && _texture.IsValid())
    {
      _device->Destroy(_texture);
    }
    _device = nullptr;
    _texture = {};
    _host = nullptr;
  }

  void ImageBufferRender2DBackend::Flush() noexcept
  {
    auto recorded = _drawingContext.TakeDisplayList();
    if (recorded->Items().empty())
    {
      return;
    }

    auto items = StatePrefix();
    items.reserve(items.size() + recorded->Items().size());
    for (const auto &item : recorded->Items())
    {
      items.push_back(item);
    }
    AdvanceState(recorded->Items());
    _pending.push_back(dl::DisplayList::Create(krys::move(items)));
  }

  List<dl::Item> ImageBufferRender2DBackend::StatePrefix() const noexcept
  {
    List<dl::Item> items;
    const boo::GraphicsContextState initial;
    const boo::GraphicsContextState *below = &initial;
    for (size_t i = 0; i < _flushedState.size(); ++i)
    {
      const auto &level = _flushedState[i];
      // A flush only happens between the canvas's calls, which end the transparency layers they begin.
      krys_debug_assert(!level.IsTransparencyLayer);
      if (i > 0)
      {
        items.emplace_back(dl::Save());
      }

      if (level.ResetsClip)
      {
        items.emplace_back(dl::ResetClip());
      }
      for (const auto &clip : level.Clips)
      {
        items.emplace_back(dl::SetCTM(clip.Ctm));
        items.push_back(clip.Clip);
      }
      items.emplace_back(dl::SetCTM(level.Ctm));

      // The level's state, as changes from the one beneath it.
      auto state = *below;
      state.DidApplyChanges();
      state.MergeAllChanges(level.State);
      if (!state.Changes().IsEmpty())
      {
        items.emplace_back(dl::SetState(state));
      }
      below = &level.State;

      if (level.LineCap)
      {
        items.emplace_back(*level.LineCap);
      }
      if (level.LineDash)
      {
        items.emplace_back(*level.LineDash);
      }
      if (level.LineJoin)
      {
        items.emplace_back(*level.LineJoin);
      }
      if (level.MiterLimit)
      {
        items.emplace_back(*level.MiterLimit);
      }
    }
    return items;
  }

  void ImageBufferRender2DBackend::AdvanceState(Span<const dl::Item> items) noexcept
  {
    auto pushLevel = [&](bool isTransparencyLayer)
    {
      auto level = _flushedState.back();
      level.ResetsClip = false;
      level.IsTransparencyLayer = isTransparencyLayer;
      level.Clips.clear();
      _flushedState.push_back(krys::move(level));
    };
    auto popLevel = [&]
    {
      if (_flushedState.size() > 1)
      {
        _flushedState.pop_back();
      }
    };

    for (const auto &item : items)
    {
      auto &level = _flushedState.back();
      std::visit(
        [&]<typename T>(const T &value)
        {
          if constexpr (SameType<T, dl::Save>)
          {
            pushLevel(false);
          }
          else if constexpr (SameType<T, dl::BeginTransparencyLayer>
                             || SameType<T, dl::BeginTransparencyLayerWithCompositeMode>)
          {
            pushLevel(true);
          }
          else if constexpr (SameType<T, dl::Restore> || SameType<T, dl::EndTransparencyLayer>)
          {
            popLevel();
          }
          else if constexpr (SameType<T, dl::Translate>)
          {
            level.Ctm.Translate(value.X(), value.Y());
          }
          else if constexpr (SameType<T, dl::Rotate>)
          {
            level.Ctm.Rotate(rad2deg(static_cast<double>(value.Angle())));
          }
          else if constexpr (SameType<T, dl::Scale>)
          {
            level.Ctm.Scale(value.Amount());
          }
          else if constexpr (SameType<T, dl::ApplyDeviceScaleFactor>)
          {
            level.Ctm.Scale(boo::FloatSize(value.ScaleFactor(), value.ScaleFactor()));
          }
          else if constexpr (SameType<T, dl::SetCTM>)
          {
            level.Ctm = value.Transform();
          }
          else if constexpr (SameType<T, dl::ConcatenateCTM>)
          {
            level.Ctm.Multiply(value.Transform());
          }
          else if constexpr (SameType<T, dl::ResetClip>)
          {
            level.ResetsClip = true;
            level.Clips.clear();
          }
          else if constexpr (SameType<T, dl::Clip> || SameType<T, dl::ClipRoundedRect>
                             || SameType<T, dl::ClipOut> || SameType<T, dl::ClipOutRoundedRect>
                             || SameType<T, dl::ClipOutToPath> || SameType<T, dl::ClipPath>
                             || SameType<T, dl::ClipToImageBuffer>)
          {
            level.Clips.push_back({.Ctm = level.Ctm, .Clip = item});
          }
          else if constexpr (SameType<T, dl::SetLineCap>)
          {
            level.LineCap = value;
          }
          else if constexpr (SameType<T, dl::SetLineDash>)
          {
            level.LineDash = value;
          }
          else if constexpr (SameType<T, dl::SetLineJoin>)
          {
            level.LineJoin = value;
          }
          else if constexpr (SameType<T, dl::SetMiterLimit>)
          {
            level.MiterLimit = value;
          }
        },
        item);
    }

    // The recorder keeps each level's whole state and transform.
    const auto &stack = _drawingContext.StateStack();
    krys_debug_assert(stack.size() == _flushedState.size());
    for (size_t i = 0; i < std::min<size_t>(stack.size(), _flushedState.size()); ++i)
    {
      _flushedState[i].State = stack[i].State;
      _flushedState[i].Ctm = stack[i].Ctm;
    }
  }

  ImageBufferSnapshot::ImageBufferSnapshot(AcceleratedImageBuffers &host, gfx::Device &device,
                                           gfx::TextureHandle texture) noexcept
      : _host(CreateWeakPtr(&host)), _device(&device), _texture(texture)
  {
  }

  ImageBufferSnapshot::~ImageBufferSnapshot() noexcept
  {
    ReleaseTexture();
  }

  gfx::TextureHandle ImageBufferSnapshot::Handle() const noexcept
  {
    return _texture;
  }

  void ImageBufferSnapshot::ReleaseTexture() noexcept
  {
    // Commands recorded but not yet run may still draw it.
    if (auto *host = _host.get(); host && _texture.IsValid())
    {
      host->Retire(_texture);
    }
    _texture = {};
    _host = nullptr;
  }

  AcceleratedImageBuffers::AcceleratedImageBuffers(FontFaceCache &fonts) noexcept : _fonts(fonts)
  {
  }

  AcceleratedImageBuffers::~AcceleratedImageBuffers() noexcept
  {
    for (auto &backend : _backends)
    {
      if (auto *live = backend.get())
      {
        live->ReleaseTexture();
      }
    }
    for (auto &snapshot : _snapshots)
    {
      if (auto *live = snapshot.get())
      {
        live->ReleaseTexture();
      }
    }
    DestroyRetiredTextures();
    _copier = nullptr;
    _imageBuffers = nullptr;
  }

  void AcceleratedImageBuffers::SetDevice(gfx::Device &device) noexcept
  {
    if (!_device)
    {
      _device = &device;
    }
    krys_debug_assert(_device == &device);
  }

  gfx::TextureHandle AcceleratedImageBuffers::RenderNow(ImageBufferRender2DBackend &backend) noexcept
  {
    if (!_device)
    {
      return {};
    }
    if (!_imageBuffers)
    {
      _imageBuffers = CreateUnique<ImageBufferTextures>(*_device, _fonts);
    }
    // Rendering outside the host's frames uses the transient memory of frames the GPU may still be reading,
    // and that the next frame reuses, so it waits for the GPU before and after.
    _device->WaitIdle();
    gfx::CommandBuffer commands;
    const auto texture = _imageBuffers->Render(backend, commands);
    _device->Submit(commands);
    _device->WaitIdle();
    _imageBuffers->Release();
    DestroyRetiredTextures();
    return texture;
  }

  void AcceleratedImageBuffers::Retire(gfx::TextureHandle texture) noexcept
  {
    _retired.push_back(texture);
  }

  void AcceleratedImageBuffers::DestroyRetiredTextures() noexcept
  {
    for (const auto texture : _retired)
    {
      _device->Destroy(texture);
    }
    _retired.clear();
  }

  RefPtr<boo::NativeImage> AcceleratedImageBuffers::Snapshot(ImageBufferRender2DBackend &backend) noexcept
  {
    const auto source = RenderNow(backend);
    if (!source.IsValid())
    {
      return nullptr;
    }
    if (!_copier)
    {
      auto renderer = gfx::Renderer2D::Create(*_device);
      if (!renderer)
      {
        return nullptr;
      }
      _copier = krys::move(*renderer);
    }

    const auto size = backend.Size();
    const auto width = static_cast<uint32>(size.Width());
    const auto height = static_cast<uint32>(size.Height());
    auto copy = _device->CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                                        .Width = width,
                                        .Height = height,
                                        .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                                        .DebugName = u8"UI.ImageBufferSnapshot"});
    if (!copy)
    {
      return nullptr;
    }

    // Premultiplied pixels drawn over transparent black are themselves.
    gfx::CommandBuffer commands;
    _copier->Begin(commands, {.Texture = *copy,
                              .Width = width,
                              .Height = height,
                              .Format = gfx::TextureFormat::RGBA8Unorm,
                              .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
    const gfx::Rect2D whole {0.f, 0.f, static_cast<float>(width), static_cast<float>(height)};
    _copier->DrawImage(source, whole, whole);
    _copier->End();
    _device->Submit(commands);
    _device->WaitIdle();

    std::erase_if(_snapshots, [](const auto &weakSnapshot) { return !weakSnapshot; });
    auto snapshot = AdoptRef(*new ImageBufferSnapshot(*this, *_device, *copy));
    _snapshots.push_back(CreateWeakPtr(snapshot.get()));
    return boo::NativeImage::Create(
      AdoptRef(*new boo::PlatformImage(size, Ref<boo::ExternalImage>(krys::move(snapshot)), true)));
  }

  RefPtr<boo::ImageBuffer> AcceleratedImageBuffers::Create(const boo::FloatSize &size,
                                                           boo::RenderingPurpose purpose,
                                                           float resolutionScale) noexcept
  {
    return boo::ImageBuffer::Create<ImageBufferRender2DBackend>(size, resolutionScale, purpose, *this);
  }

  void AcceleratedImageBuffers::Add(ImageBufferRender2DBackend &backend) noexcept
  {
    std::erase_if(_backends, [](const auto &weakBackend) { return !weakBackend; });
    _backends.push_back(CreateWeakPtr(&backend));
  }
}
