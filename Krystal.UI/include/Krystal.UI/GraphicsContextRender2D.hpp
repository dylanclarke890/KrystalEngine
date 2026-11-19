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
  /// restored on destruction. Resetting the clip brings back the renderer's clip at construction.
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

    // A clip to an image buffer's alpha: a layer that the image masks when the state it was made in ends.
    struct ImageClip
    {
      gfx::TextureHandle Texture;
      RectF Destination;
      gfx::Transform2D Transform;
      uint32 StackSize {0};
    };

    gfx::Renderer2D &_renderer;
    FontFaceCache &_fonts;
    ImageBufferTextures *_imageBuffers;
    StrokeState _stroke;
    List<StrokeState> _strokeStack;
    List<LayerState> _layerStack;
    // Clip bounds in target pixels, for `ClipBounds`.
    RectF _clipBounds;
    // The clip the context was made with, which resetting the clip brings back.
    RectF _initialClipBounds;
    gfx::Renderer2D::ClipState _initialClip;
    List<RectF> _clipBoundsStack;
    List<ImageClip> _imageClips;

  public:
    /// @param bounds The target's rect, in its pixels, the initial clip bounds.
    /// @param imageBuffers Renders the image buffers and uploads the native images drawn; without it, they
    /// draw nothing.
    GraphicsContextRender2D(gfx::Renderer2D &renderer, FontFaceCache &fonts, const RectF &bounds,
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

    void DrawRect(const RectF &, float borderThickness = 1) noexcept override;

    void DrawLine(const PointF &, const PointF &) noexcept override;

    void DrawEllipse(const RectF &) noexcept override;

    void FillPath(const boo::Path &) noexcept override;

    void StrokePath(const boo::Path &) noexcept override;

    void FillRect(const RectF &, RequiresClipToRect = RequiresClipToRect::Yes) noexcept override;

    void FillRect(const RectF &, const boo::Color &) noexcept override;

    void FillRect(const RectF &, boo::Gradient &, const boo::AffineTransform &,
                  RequiresClipToRect = RequiresClipToRect::Yes) noexcept override;

    void FillRoundedRectImpl(const boo::FloatRoundedRect &, const boo::Color &) noexcept override;

    void FillRectWithRoundedHole(const RectF &, const boo::FloatRoundedRect &roundedHoleRect,
                                 const boo::Color &) noexcept override;

    void ClearRect(const RectF &) noexcept override;

    void StrokeRect(const RectF &, float lineWidth) noexcept override;

    void SetLineCap(boo::LineCap) noexcept override;

    void SetLineDash(const boo::DashArray &, float dashOffset) noexcept override;

    void SetLineJoin(boo::LineJoin) noexcept override;

    void SetMiterLimit(float) noexcept override;

    void ResetClip() noexcept override;

    void Clip(const RectF &) noexcept override;

    void ClipRoundedRect(const boo::FloatRoundedRect &) noexcept override;

    void ClipOut(const RectF &) noexcept override;

    void ClipOut(const boo::Path &) noexcept override;

    void ClipOutRoundedRect(const boo::FloatRoundedRect &) noexcept override;

    void ClipPath(const boo::Path &, boo::WindRule = boo::WindRule::EvenOdd) noexcept override;

    KRYS_NODISCARD RectI ClipBounds() const noexcept override;

    void ClipToImageBuffer(boo::ImageBuffer &, const RectF &) noexcept override;

    void DrawGlyphs(const boo::Font &, Span<const boo::GlyphBufferGlyph>, Span<const boo::GlyphBufferAdvance>,
                    const PointF &, boo::FontSmoothingMode) noexcept override;

    void DrawLinesForText(const PointF &origin, float thickness, Span<const SegmentF> lineSegments,
                          bool isPrinting, bool doubleUnderlines, boo::StrokeStyle) noexcept override;

    void DrawDotsForDocumentMarker(const RectF &, boo::DocumentMarkerLineStyle) noexcept override;

    void BeginTransparencyLayer(float opacity) noexcept override;

    void BeginTransparencyLayer(boo::CompositeOperator, boo::BlendMode = BlendMode::Normal) noexcept override;

    void EndTransparencyLayer() noexcept override;

    void DrawNativeImage(boo::NativeImage &, const RectF &destRect, const RectF &srcRect,
                         boo::ImagePaintingOptions = {}) noexcept override;

    void DrawPattern(boo::NativeImage &, const RectF &destRect, const RectF &tileRect,
                     const boo::AffineTransform &patternTransform, const PointF &phase, const SizeF &spacing,
                     boo::ImagePaintingOptions = {}) noexcept override;

    void DrawImageBuffer(boo::ImageBuffer &, const RectF &destination, const RectF &source,
                         boo::ImagePaintingOptions = {}) noexcept override;

    void DrawPattern(boo::ImageBuffer &, const RectF &destRect, const RectF &tileRect,
                     const boo::AffineTransform &patternTransform, const PointF &phase, const SizeF &spacing,
                     boo::ImagePaintingOptions = {}) noexcept override;

    /// @brief Applies @p filter on the GPU: its effects become a Render2D filter graph, evaluated into the
    /// replay's command buffer ahead of its passes.
    void DrawFilteredImageBuffer(boo::ImageBuffer *sourceImage, const RectF &sourceImageRect,
                                 boo::Filter &filter, boo::FilterResults &results) noexcept override;

    void DrawFocusRing(const boo::Path &, float outlineWidth, const boo::Color &) noexcept override;

    void DrawFocusRing(const List<RectF> &, float outlineOffset, float outlineWidth,
                       const boo::Color &) noexcept override;

    void Scale(const SizeF &) noexcept override;

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
      Maybe<gfx::ImagePattern2D> Pattern;

      KRYS_NODISCARD gfx::Paint2D Paint() const noexcept
      {
        if (Pattern)
        {
          return gfx::Paint2D(*Pattern);
        }
        return Gradient ? gfx::Paint2D(*Gradient, GradientSpace) : gfx::Paint2D(Colour);
      }
    };

    KRYS_NODISCARD Brush FillBrushPaint() const noexcept;

    /// @brief A brush's pattern as a repeated image: its tile rendered into a texture; a transparent
    /// colour when there's no texture.
    KRYS_NODISCARD Brush PatternBrushPaint(const boo::Pattern &) const noexcept;

    KRYS_NODISCARD Brush StrokeBrushPaint() const noexcept;

    KRYS_NODISCARD gfx::StrokeStyle CurrentStrokeStyle(float width) const noexcept;

    /// @brief How images are sampled at the context's interpolation quality, as Skia's sampling options.
    KRYS_NODISCARD gfx::SamplerFilter ImageFilter() const noexcept;

    /// @brief Draws @p source of @p texture into @p destination, as an image is drawn.
    void DrawTexture(gfx::TextureHandle texture, const RectF &source, const RectF &destination,
                     boo::ImagePaintingOptions options,
                     AlphaPremultiplication alpha = AlphaPremultiplication::Premultiplied) noexcept;

    /// @brief Fills @p destRect with @p tileRect of @p texture (of @p size), repeated, as an image pattern.
    void DrawTexturePattern(gfx::TextureHandle texture, const SizeF &size, const RectF &destRect,
                            const RectF &tileRect, const boo::AffineTransform &patternTransform,
                            const PointF &phase, const SizeF &spacing, boo::ImagePaintingOptions options,
                            gfx::CommandBuffer &commands) noexcept;

    /// @brief Brings the renderer's blend mode and drop shadow up to date with the context's state.
    void PrepareDraw(bool shadow = true) noexcept;

    /// @brief Sets an image draw's blend mode and operator from @p options.
    void PrepareImageDraw(const boo::ImagePaintingOptions &options) noexcept;

    void FillShape(const gfx::Path2D &path, gfx::FillRule rule, const gfx::Paint2D &paint) noexcept;

    void IntersectClipBounds(const RectF &rect) noexcept;

    void EndImageClips() noexcept;
  };
}
