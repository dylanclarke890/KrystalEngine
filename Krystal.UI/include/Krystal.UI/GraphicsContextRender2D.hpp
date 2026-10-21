#pragma once

#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/FontFaceCache.hpp"
#include "Krystal.UI/ImageBufferTextures.hpp"

namespace krys::ui
{
  /// @brief A graphics context that draws through Render2D, so display lists recorded by Booey replay into a
  /// window or texture. Drawing follows the Skia backend's: brushes become paints multiplied by the context's
  /// alpha, drop shadows and blend modes apply to each draw, and lines, rects and text lines are drawn as
  /// Skia draws them.
  /// @note Draws between `Renderer2D::Begin` and `End`; the renderer's state is saved on construction and
  /// restored on destruction.
  /// FIXME: Compositing operators other than source-over (`Copy`, `Clear` and the rest, `clearRect`) and
  /// resetting the clip need Render2D support; they draw as source-over and do nothing.
  class GraphicsContextRender2D final : public boo::GraphicsContext
  {
  private:
    struct StrokeState
    {
      gfx::LineCap Cap {gfx::LineCap::Butt};
      gfx::LineJoin Join {gfx::LineJoin::Miter};
      float MiterLimit {4.f};
      List<float> Dashes;
      float DashOffset {0.f};
    };

    struct LayerState
    {
      Maybe<boo::CompositeMode> CompositeMode;
    };

    gfx::Renderer2D &_renderer;
    FontFaceCache &_fonts;
    ImageBufferTextures *_imageBuffers;
    StrokeState _stroke;
    List<StrokeState> _strokeStack;
    List<LayerState> _layerStack;
    // Clip bounds in target pixels, for `ClipBounds`.
    boo::FloatRect _clipBounds;
    List<boo::FloatRect> _clipBoundsStack;

  public:
    /// @param bounds The target's rect, in its pixels, the initial clip bounds.
    /// @param imageBuffers Renders the image buffers and uploads the native images drawn; without it, they
    /// draw nothing.
    GraphicsContextRender2D(gfx::Renderer2D &renderer, FontFaceCache &fonts, const boo::FloatRect &bounds,
                            ImageBufferTextures *imageBuffers = nullptr) noexcept;
    ~GraphicsContextRender2D() noexcept override;

    using GraphicsContext::FillRect;
    using GraphicsContext::Scale;
    using GraphicsContext::Translate;

    void DidUpdateState(boo::GraphicsContextState &) noexcept override
    {
    }
    void DidUpdateSingleState(boo::GraphicsContextState &,
                              boo::GraphicsContextState::ChangeIndex) noexcept override
    {
    }

    void Save(
      boo::GraphicsContextState::Purpose = boo::GraphicsContextState::Purpose::SaveRestore) noexcept override;
    void Restore(
      boo::GraphicsContextState::Purpose = boo::GraphicsContextState::Purpose::SaveRestore) noexcept override;

    void DrawRect(const boo::FloatRect &, float borderThickness = 1) noexcept override;
    void DrawLine(const boo::FloatPoint &, const boo::FloatPoint &) noexcept override;
    void DrawEllipse(const boo::FloatRect &) noexcept override;

    void FillPath(const boo::Path &) noexcept override;
    void StrokePath(const boo::Path &) noexcept override;

    void FillRect(const boo::FloatRect &, RequiresClipToRect = RequiresClipToRect::Yes) noexcept override;
    void FillRect(const boo::FloatRect &, const boo::Color &) noexcept override;
    void FillRect(const boo::FloatRect &, boo::Gradient &, const boo::AffineTransform &,
                  RequiresClipToRect = RequiresClipToRect::Yes) noexcept override;
    void FillRoundedRectImpl(const boo::FloatRoundedRect &, const boo::Color &) noexcept override;
    void FillRectWithRoundedHole(const boo::FloatRect &, const boo::FloatRoundedRect &roundedHoleRect,
                                 const boo::Color &) noexcept override;

    void ClearRect(const boo::FloatRect &) noexcept override;
    void StrokeRect(const boo::FloatRect &, float lineWidth) noexcept override;

    void SetLineCap(boo::LineCap) noexcept override;
    void SetLineDash(const boo::DashArray &, float dashOffset) noexcept override;
    void SetLineJoin(boo::LineJoin) noexcept override;
    void SetMiterLimit(float) noexcept override;

    void ResetClip() noexcept override;
    void Clip(const boo::FloatRect &) noexcept override;
    void ClipRoundedRect(const boo::FloatRoundedRect &) noexcept override;
    void ClipOut(const boo::FloatRect &) noexcept override;
    void ClipOut(const boo::Path &) noexcept override;
    void ClipOutRoundedRect(const boo::FloatRoundedRect &) noexcept override;
    void ClipPath(const boo::Path &, boo::WindRule = boo::WindRule::EvenOdd) noexcept override;
    KRYS_NODISCARD boo::IntRect ClipBounds() const noexcept override;

    void DrawGlyphs(const boo::Font &, Span<const boo::GlyphBufferGlyph>, Span<const boo::GlyphBufferAdvance>,
                    const boo::FloatPoint &, boo::FontSmoothingMode) noexcept override;
    void DrawLinesForText(const boo::FloatPoint &origin, float thickness,
                          Span<const boo::FloatSegment> lineSegments, bool isPrinting, bool doubleLines,
                          boo::StrokeStyle) noexcept override;
    void DrawDotsForDocumentMarker(const boo::FloatRect &, boo::DocumentMarkerLineStyle) noexcept override;

    void BeginTransparencyLayer(float opacity) noexcept override;
    void BeginTransparencyLayer(boo::CompositeOperator, boo::BlendMode = BlendMode::Normal) noexcept override;
    void EndTransparencyLayer() noexcept override;

    void DrawNativeImage(boo::NativeImage &, const boo::FloatRect &destRect, const boo::FloatRect &srcRect,
                         boo::ImagePaintingOptions = {}) noexcept override;
    void DrawPattern(boo::NativeImage &, const boo::FloatRect &destRect, const boo::FloatRect &tileRect,
                     const boo::AffineTransform &patternTransform, const boo::FloatPoint &phase,
                     const boo::FloatSize &spacing, boo::ImagePaintingOptions = {}) noexcept override;
    void DrawImageBuffer(boo::ImageBuffer &, const boo::FloatRect &destination, const boo::FloatRect &source,
                         boo::ImagePaintingOptions = {}) noexcept override;
    void DrawPattern(boo::ImageBuffer &, const boo::FloatRect &destRect, const boo::FloatRect &tileRect,
                     const boo::AffineTransform &patternTransform, const boo::FloatPoint &phase,
                     const boo::FloatSize &spacing, boo::ImagePaintingOptions = {}) noexcept override;

    void DrawFocusRing(const boo::Path &, float outlineWidth, const boo::Color &) noexcept override;
    void DrawFocusRing(const List<boo::FloatRect> &, float outlineOffset, float outlineWidth,
                       const boo::Color &) noexcept override;

    void Scale(const boo::FloatSize &) noexcept override;
    void Rotate(float angleInRadians) noexcept override;
    void Translate(float x, float y) noexcept override;
    void ConcatCTM(const boo::AffineTransform &) noexcept override;
    void SetCTM(const boo::AffineTransform &) noexcept override;
    KRYS_NODISCARD boo::AffineTransform
      GetCTM(IncludeDeviceScale = PossiblyIncludeDeviceScale) const noexcept override;

  private:
    /// @brief A fill or stroke's paint, keeping the gradient it refers to alive.
    struct Brush
    {
      SRGBA<uint8> Colour {};
      Maybe<gfx::Gradient2D> Gradient;
      gfx::Transform2D GradientSpace {};

      KRYS_NODISCARD gfx::Paint2D Paint() const noexcept
      {
        return Gradient ? gfx::Paint2D(*Gradient, GradientSpace) : gfx::Paint2D(Colour);
      }
    };

    KRYS_NODISCARD Brush FillBrushPaint() const noexcept;
    KRYS_NODISCARD Brush StrokeBrushPaint() const noexcept;
    KRYS_NODISCARD gfx::StrokeStyle CurrentStrokeStyle(float width) const noexcept;

    /// @brief How images are sampled at the context's interpolation quality, as Skia's sampling options.
    KRYS_NODISCARD gfx::SamplerFilter ImageFilter() const noexcept;

    /// @brief Draws @p source of @p texture into @p destination, as an image is drawn.
    void DrawTexture(gfx::TextureHandle texture, const boo::FloatRect &source,
                     const boo::FloatRect &destination, boo::ImagePaintingOptions options) noexcept;
    /// @brief Fills @p destRect with @p tileRect of @p texture (of @p size), repeated, as an image pattern.
    void DrawTexturePattern(gfx::TextureHandle texture, const boo::FloatSize &size,
                            const boo::FloatRect &destRect, const boo::FloatRect &tileRect,
                            const boo::AffineTransform &patternTransform, const boo::FloatPoint &phase,
                            const boo::FloatSize &spacing, boo::ImagePaintingOptions options,
                            gfx::CommandBuffer &commands) noexcept;

    /// @brief Brings the renderer's blend mode and drop shadow up to date with the context's state.
    void PrepareDraw(bool shadow = true) noexcept;
    /// @brief Sets an image draw's blend mode and operator from @p options.
    void PrepareImageDraw(const boo::ImagePaintingOptions &options) noexcept;

    void FillShape(const gfx::Path2D &path, gfx::FillRule rule, const gfx::Paint2D &paint) noexcept;
    void IntersectClipBounds(const boo::FloatRect &rect) noexcept;
  };
}
