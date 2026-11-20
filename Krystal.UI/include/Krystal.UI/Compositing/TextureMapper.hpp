#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayerEnums.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Geometry/TransformationMatrix.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/Compositing/BitmapTexturePool.hpp"

namespace krys::boo
{
  class FilterOperations;
  class GraphicsLayer;
  class Image;
}

namespace krys::ui
{
  class FontFaceCache;
  class ImageBufferRender2DBackend;
  class ImageBufferTextures;

  /// @brief Draws a composited layer tree with Render2D: the layers' textures, solid colours and borders,
  /// through clips and groups (transparency layers with filters), and paints layers' contents into their
  /// textures.
  class TextureMapper
  {
    KRYS_NON_COPY_MOVABLE(TextureMapper);

  public:
    enum class WrapMode : uint8
    {
      Stretch,
      Repeat,
    };

    enum class AllEdgesExposed : bool
    {
      No,
      Yes,
    };

  private:
    gfx::Device &_device;
    FontFaceCache &_fonts;
    /// @brief Paints layers' contents into their textures.
    UniquePtr<gfx::Renderer2D> _contentsRenderer;
    UniquePtr<ImageBufferTextures> _imageBuffers;
    BitmapTexturePool _texturePool;

    /// @brief The frame's renderer, between `BeginPainting` and `EndPainting`.
    gfx::Renderer2D *_renderer {nullptr};
    /// @brief The renderer's transform when painting began: where the host draws the view.
    AffineTransform _baseTransform;
    List<RectI> _clipBounds;
    /// @brief The Render2D layers each group began.
    List<uint32> _groupLayerCounts;

    WrapMode _wrapMode {WrapMode::Stretch};
    TransformationMatrix _patternTransform;
    boo::GraphicsLayerScalingFilter _minificationFilter {boo::GraphicsLayerScalingFilter::Linear};
    boo::GraphicsLayerScalingFilter _magnificationFilter {boo::GraphicsLayerScalingFilter::Linear};

  public:
    KRYS_NODISCARD static Expected<UniquePtr<TextureMapper>> Create(gfx::Device &device,
                                                                    FontFaceCache &fonts) noexcept;

    ~TextureMapper() noexcept;

    KRYS_NODISCARD gfx::Device &Device() const noexcept
    {
      return _device;
    }

    /// @brief Starts drawing into the frame `renderer` draws, whose clip is `bounds` (in the view's
    /// coordinates). Layers are drawn under the renderer's current transform.
    void BeginPainting(gfx::Renderer2D &renderer, const RectI &bounds) noexcept;

    void EndPainting() noexcept;

    void DrawBorder(const boo::Color &color, float borderWidth, const RectF &targetRect,
                    const TransformationMatrix &modelViewMatrix) noexcept;

    void DrawNumber(int32 number, const boo::Color &color, const PointF &targetPoint,
                    const TransformationMatrix &modelViewMatrix) noexcept;

    void DrawTexture(const BitmapTexture &texture, const RectF &target,
                     const TransformationMatrix &modelViewMatrix = {}, float opacity = 1.0f,
                     AllEdgesExposed allEdgesExposed = AllEdgesExposed::Yes) noexcept;

    /// @brief Draws a texture of @p textureSize the mapper doesn't own, whose alpha is @p alpha.
    void DrawTexture(gfx::TextureHandle texture, const SizeI &textureSize, const RectF &target,
                     const TransformationMatrix &modelViewMatrix, float opacity,
                     AlphaPremultiplication alpha) noexcept;

    /// @brief The texture an accelerated image buffer keeps, with what's been flushed rendered into it ahead
    /// of the frame's passes; invalid if it can't be. Between `BeginPainting` and `EndPainting`.
    KRYS_NODISCARD gfx::TextureHandle RenderImageBuffer(ImageBufferRender2DBackend &backend) noexcept;

    void DrawSolidColor(const RectF &rect, const TransformationMatrix &matrix, const boo::Color &color,
                        bool isBlendingAllowed) noexcept;

    void BeginClip(const TransformationMatrix &modelViewMatrix, const boo::FloatRoundedRect &rect) noexcept;

    void EndClip() noexcept;

    KRYS_NODISCARD RectI ClipBounds() const noexcept;

    /// @brief Starts drawing into a group composited at `opacity` with `blendMode` through `filters` (the
    /// last outermost), as an intermediate surface would be. Blurs read @p blurEdgeMode past the group's
    /// clip: a backdrop's duplicate its edges, as a surface the size of the clip would.
    /// @brief In mask mode, what's drawn masks what's beneath it (`destination-in`): a mask layer applied to
    /// its layer's group.
    void SetMaskMode(bool maskMode) noexcept;

    void BeginGroup(float opacity, BlendMode blendMode, const boo::FilterOperations &filters,
                    gfx::EdgeMode2D blurEdgeMode = gfx::EdgeMode2D::None) noexcept;

    void EndGroup() noexcept;

    KRYS_NODISCARD SizeI MaxTextureSize() const noexcept;

    void SetWrapMode(WrapMode mode) noexcept
    {
      _wrapMode = mode;
    }

    /// @brief How the textures drawn next are sampled where they're shown smaller and larger than they are.
    void SetScalingFilters(boo::GraphicsLayerScalingFilter minification,
                           boo::GraphicsLayerScalingFilter magnification) noexcept
    {
      _minificationFilter = minification;
      _magnificationFilter = magnification;
    }

    void SetPatternTransform(const TransformationMatrix &transform) noexcept
    {
      _patternTransform = transform;
    }

    KRYS_NODISCARD Ref<BitmapTexture> AcquireTextureFromPool(const SizeI &size,
                                                             EnumSet<BitmapTexture::Flags> flags) noexcept;

    /// @brief Paints `sourceLayer`'s contents at `sourceOffset` (scaled by `scale`) into `targetRect` of
    /// `texture`, replacing what was there.
    void UpdateContents(BitmapTexture &texture, const boo::GraphicsLayer &sourceLayer,
                        const RectI &targetRect, const PointI &sourceOffset, float scale = 1) noexcept;

    /// @brief Draws `image` at `sourceOffset` into `targetRect` of `texture`, replacing what was there.
    void UpdateContents(BitmapTexture &texture, boo::Image &image, const RectI &targetRect,
                        const PointI &sourceOffset) noexcept;

  private:
    TextureMapper(gfx::Device &device, FontFaceCache &fonts,
                  UniquePtr<gfx::Renderer2D> &&contentsRenderer) noexcept;

    /// @brief Draws under `matrix`, from the view's coordinates, after the base transform.
    void SetTransform(const TransformationMatrix &matrix) noexcept;

    /// @brief How a texture of @p textureSize drawn into @p targetRect is sampled.
    KRYS_NODISCARD gfx::SamplerFilter
      ScalingFilter(const SizeI &textureSize, const RectF &targetRect,
                    const TransformationMatrix &modelViewMatrix) const noexcept;

    template <typename TPaint>
    void PaintIntoTexture(BitmapTexture &texture, const RectI &targetRect, TPaint paint) noexcept;
  };
}
