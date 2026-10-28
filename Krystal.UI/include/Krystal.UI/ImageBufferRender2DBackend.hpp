#pragma once

#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/ImageBufferBackend.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.UI/Compositing/PlatformLayerDisplayDelegate.hpp"
#include "Krystal.UI/TextureImage.hpp"

namespace krys::boo
{
  class ImageBuffer;
}

namespace krys::gfx
{
  class CommandBuffer;
  class Device;
  class Renderer2D;
}

namespace krys::boo
{
  class NativeImage;
}

namespace krys::ui
{
  class AcceleratedImageBuffers;
  class FontFaceCache;
  class ImageBufferRender2DBackend;
  class ImageBufferTextures;

  /// @brief Draws an accelerated image buffer's texture as a layer's contents, rendering what was flushed
  /// into it first.
  class ImageBufferRender2DLayer final : public TextureMapperPlatformLayer
  {
  private:
    WeakPtr<ImageBufferRender2DBackend> _backend;

  public:
    explicit ImageBufferRender2DLayer(ImageBufferRender2DBackend &backend) noexcept;

    void PaintToTextureMapper(TextureMapper &textureMapper, const boo::FloatRect &targetRect,
                              const boo::TransformationMatrix &modelViewMatrix = {},
                              float opacity = 1.0) noexcept override;
  };

  /// @brief Shows an accelerated image buffer's texture as a layer's contents; it shows nothing once the
  /// buffer is gone.
  class ImageBufferRender2DDisplayDelegate final : public PlatformLayerDisplayDelegate
  {
  private:
    ImageBufferRender2DLayer _layer;

  public:
    KRYS_NODISCARD static Ref<ImageBufferRender2DDisplayDelegate>
      Create(ImageBufferRender2DBackend &backend) noexcept
    {
      return AdoptRef(*new ImageBufferRender2DDisplayDelegate(backend));
    }

  private:
    explicit ImageBufferRender2DDisplayDelegate(ImageBufferRender2DBackend &backend) noexcept
        : PlatformLayerDisplayDelegate(&_layer), _layer(backend)
    {
    }
  };

  /// @brief A copy of an accelerated image buffer's texture, as it was when the copy was made: what a canvas
  /// drawn into a canvas, or made into a pattern, draws.
  /// @note Its texture is freed once the commands that may draw it have run after it goes, or with the view
  /// whose page made the buffer; it draws nothing after.
  class ImageBufferSnapshot final : public TextureImage, public CanMakeWeakPtr<ImageBufferSnapshot>
  {
    friend class AcceleratedImageBuffers;

  private:
    WeakPtr<AcceleratedImageBuffers> _host;
    gfx::Device *_device;
    gfx::TextureHandle _texture;

  public:
    ImageBufferSnapshot(AcceleratedImageBuffers &host, gfx::Device &device,
                        gfx::TextureHandle texture) noexcept;
    ~ImageBufferSnapshot() noexcept override;

    KRYS_NODISCARD gfx::TextureHandle Handle() const noexcept override;

  private:
    void ReleaseTexture() noexcept;
  };

  /// @brief An accelerated image buffer's backend: what's drawn is recorded, and each flush hands what was
  /// recorded since the last to be rendered into a texture the backend keeps, without clearing it.
  /// @note What's flushed is rendered the next time the buffer is drawn: by the compositor, before it
  /// composites (the rendering update flushes it as it prepares canvases for display), or when the buffer
  /// is painted, which flushes everything drawn so far. Each flush's drawing starts with the state the
  /// last one ended with: the state stack, transforms, clips and line styles.
  /// @note The texture is made the first time the buffer is rendered, from the renderer's device, and freed
  /// with the backend or with the view whose page made it; the buffer is transparent black until then.
  /// @note Reading the pixels back, or copying them into an image, renders what's been drawn there and then,
  /// once the view has a device (`View::SetDevice`, or its first paint); before that, they're transparent
  /// black and there's no image. Writing pixels draws them, in order with the rest.
  /// FIXME: Buffers of a resolution scale other than 1 replay their transforms unscaled, as the display list
  /// backend does; canvases' buffers are 1.
  class ImageBufferRender2DBackend final : public boo::ImageBufferBackend,
                                           public CanMakeWeakPtr<ImageBufferRender2DBackend>,
                                           public CanMakeCheckedPtr<ImageBufferRender2DBackend>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ImageBufferRender2DBackend);

  private:
    /// @brief A recorder whose state stack can be read: each level's state and transform. Its compatible
    /// buffers are accelerated, and it draws an accelerated buffer as a copy of it, made at the call.
    class Recorder final : public boo::display_list::RecorderImpl
    {
    private:
      ImageBufferRender2DBackend &_backend;

    public:
      using boo::display_list::Recorder::ContextState;
      using boo::display_list::Recorder::StateStack;

      Recorder(const boo::FloatSize &size, ImageBufferRender2DBackend &backend) noexcept
          : RecorderImpl(size), _backend(backend)
      {
      }

      KRYS_NODISCARD RefPtr<boo::ImageBuffer>
        CreateImageBuffer(const boo::FloatSize &, float resolutionScale = 1) const noexcept override;
      void DrawImageBuffer(boo::ImageBuffer &, const boo::FloatRect &destRect, const boo::FloatRect &srcRect,
                           boo::ImagePaintingOptions) noexcept override;
      using RecorderImpl::DrawImageBuffer;
    };

    /// @brief A clip, under the transform it was made under.
    struct ClipEntry
    {
      boo::AffineTransform Ctm;
      boo::display_list::Item Clip;
    };

    /// @brief A level of the state stack as a flush ended it: what the next flush's drawing starts with.
    struct StateLevel
    {
      boo::GraphicsContextState State;
      boo::AffineTransform Ctm;
      /// @brief Whether the level reset the clip of the levels beneath it.
      bool ResetsClip {false};
      bool IsTransparencyLayer {false};
      List<ClipEntry> Clips;
      Maybe<boo::display_list::SetLineCap> LineCap;
      Maybe<boo::display_list::SetLineDash> LineDash;
      Maybe<boo::display_list::SetLineJoin> LineJoin;
      Maybe<boo::display_list::SetMiterLimit> MiterLimit;
    };

    Recorder _drawingContext;
    WeakPtr<AcceleratedImageBuffers> _host;
    Ref<ImageBufferRender2DDisplayDelegate> _delegate;
    /// @brief The state stack at the end of what's been flushed.
    List<StateLevel> _flushedState;
    /// @brief What's been flushed and not yet rendered, each starting with the state it was drawn in.
    List<Ref<const boo::display_list::DisplayList>> _pending;
    gfx::Device *_device {nullptr};
    gfx::TextureHandle _texture;

  public:
    static constexpr boo::RenderingMode BackendRenderingMode = boo::RenderingMode::Accelerated;

    KRYS_NODISCARD static UniquePtr<ImageBufferRender2DBackend>
      Create(const Parameters &parameters, AcceleratedImageBuffers &host) noexcept;

    ImageBufferRender2DBackend(const Parameters &parameters, AcceleratedImageBuffers &host) noexcept;
    ~ImageBufferRender2DBackend() noexcept override;

    KRYS_NODISCARD boo::GraphicsContext &Context() noexcept override;
    void FlushContext() noexcept override;
    void PrepareForDisplay() noexcept override;
    KRYS_NODISCARD RefPtr<boo::GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() const noexcept override;

    KRYS_NODISCARD RefPtr<boo::NativeImage> CopyNativeImage() noexcept override;
    KRYS_NODISCARD RefPtr<boo::NativeImage> CreateNativeImageReference() noexcept override;
    using ImageBufferBackend::GetPixelBuffer;
    void GetPixelBuffer(const boo::IntRect &srcRect, boo::PixelBuffer &destination) noexcept override;

    /// @brief Renders what's been flushed into the texture with @p renderer, recording into @p commands,
    /// and returns the texture; invalid if it can't be made.
    KRYS_NODISCARD gfx::TextureHandle Render(gfx::Renderer2D &renderer, FontFaceCache &fonts,
                                             ImageBufferTextures &imageBuffers,
                                             gfx::CommandBuffer &commands) noexcept;

    /// @brief Whether there's flushed drawing waiting to be rendered.
    KRYS_NODISCARD bool HasPendingDrawing() const noexcept
    {
      return !_pending.empty();
    }

    /// @brief Frees the texture, as the view whose page made the buffer goes; the buffer draws nothing
    /// after.
    void ReleaseTexture() noexcept;

  private:
    /// @brief Hands what's been recorded since the last flush to be rendered.
    void Flush() noexcept;
    /// @brief The items that bring a fresh context to the state the last flush ended with.
    KRYS_NODISCARD List<boo::display_list::Item> StatePrefix() const noexcept;
    /// @brief Follows @p items' changes to the state stack.
    void AdvanceState(Span<const boo::display_list::Item> items) noexcept;
  };

  /// @brief The accelerated image buffers a view's pages make (the canvases' bitmaps): their backends are
  /// Render2D's, and their textures are freed when the view goes.
  class AcceleratedImageBuffers final : public CanMakeWeakPtr<AcceleratedImageBuffers>,
                                        public CanMakeCheckedPtr<AcceleratedImageBuffers>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(AcceleratedImageBuffers);
    KRYS_NON_COPY_MOVABLE(AcceleratedImageBuffers);

  private:
    FontFaceCache &_fonts;
    List<WeakPtr<ImageBufferRender2DBackend>> _backends;
    List<WeakPtr<ImageBufferSnapshot>> _snapshots;
    /// @brief The textures of snapshots that are gone, which recorded commands may still draw.
    List<gfx::TextureHandle> _retired;
    gfx::Device *_device {nullptr};
    /// @brief Renders buffers outside a paint, made with the device.
    UniquePtr<ImageBufferTextures> _imageBuffers;
    UniquePtr<gfx::Renderer2D> _copier;

  public:
    explicit AcceleratedImageBuffers(FontFaceCache &fonts) noexcept;
    ~AcceleratedImageBuffers() noexcept;

    /// @brief Gives the buffers the device their textures are made on, so that they can be read back and
    /// copied before the view first paints; the first device the view is given or painted with stays.
    void SetDevice(gfx::Device &device) noexcept;
    KRYS_NODISCARD gfx::Device *Device() const noexcept
    {
      return _device;
    }

    /// @brief Renders what @p backend has flushed into its texture now, and returns the texture; invalid if
    /// there's no device yet.
    KRYS_NODISCARD gfx::TextureHandle RenderNow(ImageBufferRender2DBackend &backend) noexcept;
    /// @brief A copy of @p backend's texture with everything flushed rendered into it; null if there's no
    /// device yet.
    KRYS_NODISCARD RefPtr<boo::NativeImage> Snapshot(ImageBufferRender2DBackend &backend) noexcept;

    /// @brief Called by a snapshot as it goes: its texture is freed once the commands recorded so far have
    /// run.
    void Retire(gfx::TextureHandle texture) noexcept;
    /// @brief Frees the textures of the snapshots that are gone, once the commands that may draw them have
    /// run: as the view starts painting again, the host having submitted its last frame.
    void DestroyRetiredTextures() noexcept;

    /// @brief An accelerated buffer of @p size logical units; null if it's empty.
    KRYS_NODISCARD RefPtr<boo::ImageBuffer> Create(const boo::FloatSize &size, boo::RenderingPurpose purpose,
                                                   float resolutionScale) noexcept;

    /// @brief Called by a backend as it's made.
    void Add(ImageBufferRender2DBackend &backend) noexcept;
  };
}
