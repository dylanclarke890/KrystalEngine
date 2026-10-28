#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
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
      Repeat
    };

    enum class AllEdgesExposed : bool
    {
      No,
      Yes
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
    gfx::Transform2D _baseTransform;
    List<boo::IntRect> _clipBounds;
    /// @brief The Render2D layers each group began.
    List<uint32> _groupLayerCounts;

    WrapMode _wrapMode {WrapMode::Stretch};
    boo::TransformationMatrix _patternTransform;

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
    void BeginPainting(gfx::Renderer2D &renderer, const boo::IntRect &bounds) noexcept;
    void EndPainting() noexcept;

    void DrawBorder(const boo::Color &color, float borderWidth, const boo::FloatRect &targetRect,
                    const boo::TransformationMatrix &modelViewMatrix) noexcept;
    void DrawNumber(int32 number, const boo::Color &color, const boo::FloatPoint &targetPoint,
                    const boo::TransformationMatrix &modelViewMatrix) noexcept;

    void DrawTexture(const BitmapTexture &texture, const boo::FloatRect &target,
                     const boo::TransformationMatrix &modelViewMatrix = {}, float opacity = 1.0f,
                     AllEdgesExposed allEdgesExposed = AllEdgesExposed::Yes) noexcept;
    /// @brief Draws a texture the mapper doesn't own, whose alpha is @p alpha.
    void DrawTexture(gfx::TextureHandle texture, const boo::FloatRect &target,
                     const boo::TransformationMatrix &modelViewMatrix, float opacity,
                     AlphaPremultiplication alpha) noexcept;
    void DrawSolidColor(const boo::FloatRect &rect, const boo::TransformationMatrix &matrix,
                        const boo::Color &color, bool isBlendingAllowed) noexcept;

    void BeginClip(const boo::TransformationMatrix &modelViewMatrix,
                   const boo::FloatRoundedRect &rect) noexcept;
    void EndClip() noexcept;
    KRYS_NODISCARD boo::IntRect ClipBounds() const noexcept;

    /// @brief Starts drawing into a group composited at `opacity` with `blendMode` through `filters` (the
    /// last outermost), as an intermediate surface would be. Blurs read @p blurEdgeMode past the group's
    /// clip: a backdrop's duplicate its edges, as a surface the size of the clip would.
    /// @brief In mask mode, what's drawn masks what's beneath it (`destination-in`): a mask layer applied to
    /// its layer's group.
    void SetMaskMode(bool maskMode) noexcept;

    void BeginGroup(float opacity, BlendMode blendMode, const boo::FilterOperations &filters,
                    gfx::EdgeMode2D blurEdgeMode = gfx::EdgeMode2D::None) noexcept;
    void EndGroup() noexcept;

    KRYS_NODISCARD boo::IntSize MaxTextureSize() const noexcept;

    void SetWrapMode(WrapMode mode) noexcept
    {
      _wrapMode = mode;
    }
    void SetPatternTransform(const boo::TransformationMatrix &transform) noexcept
    {
      _patternTransform = transform;
    }

    KRYS_NODISCARD Ref<BitmapTexture> AcquireTextureFromPool(const boo::IntSize &size,
                                                             EnumSet<BitmapTexture::Flags> flags) noexcept;

    /// @brief Paints `sourceLayer`'s contents at `sourceOffset` (scaled by `scale`) into `targetRect` of
    /// `texture`, replacing what was there.
    void UpdateContents(BitmapTexture &texture, const boo::GraphicsLayer &sourceLayer,
                        const boo::IntRect &targetRect, const boo::IntPoint &sourceOffset,
                        float scale = 1) noexcept;
    /// @brief Draws `image` at `sourceOffset` into `targetRect` of `texture`, replacing what was there.
    void UpdateContents(BitmapTexture &texture, boo::Image &image, const boo::IntRect &targetRect,
                        const boo::IntPoint &sourceOffset) noexcept;

  private:
    TextureMapper(gfx::Device &device, FontFaceCache &fonts,
                  UniquePtr<gfx::Renderer2D> &&contentsRenderer) noexcept;

    /// @brief Draws under `matrix`, from the view's coordinates, after the base transform.
    void SetTransform(const boo::TransformationMatrix &matrix) noexcept;

    template <typename TPaint>
    void PaintIntoTexture(BitmapTexture &texture, const boo::IntRect &targetRect, TPaint &&paint) noexcept;
  };
}
