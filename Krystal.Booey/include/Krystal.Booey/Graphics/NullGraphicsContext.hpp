#pragma once

#include "Krystal.Booey/Graphics/GraphicsContext.hpp"

namespace krys::boo
{
  enum class NullGraphicsContextPaintInvalidationReasons : uint8
  {
    None,
    InvalidatingControlTints,
    InvalidatingImagesWithAsyncDecodes,
    DetectingContentfulPaint
  };

  /// @brief A graphics context that draws nothing, for painting that only computes (invalidation, contentful
  /// paint detection).
  class NullGraphicsContext : public GraphicsContext
  {
  public:
    using PaintInvalidationReasons = NullGraphicsContextPaintInvalidationReasons;

  private:
    const PaintInvalidationReasons _paintInvalidationReasons {PaintInvalidationReasons::None};

  public:
    NullGraphicsContext() noexcept = default;
    NullGraphicsContext(PaintInvalidationReasons reasons) noexcept : _paintInvalidationReasons(reasons)
    {
    }

  private:
    KRYS_NODISCARD bool PaintingDisabled() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD bool PerformingPaintInvalidation() const noexcept final
    {
      return _paintInvalidationReasons != PaintInvalidationReasons::None;
    }
    KRYS_NODISCARD bool InvalidatingControlTints() const noexcept final
    {
      return _paintInvalidationReasons == PaintInvalidationReasons::InvalidatingControlTints;
    }
    KRYS_NODISCARD bool InvalidatingImagesWithAsyncDecodes() const noexcept final
    {
      return _paintInvalidationReasons == PaintInvalidationReasons::InvalidatingImagesWithAsyncDecodes;
    }
    KRYS_NODISCARD bool DetectingContentfulPaint() const noexcept final
    {
      return _paintInvalidationReasons == PaintInvalidationReasons::DetectingContentfulPaint;
    }

    void DidUpdateState(GraphicsContextState &) noexcept final
    {
    }

    KRYS_NODISCARD IntRect ClipBounds() const noexcept final
    {
      return {};
    }

    void DrawRect(const FloatRect &, float = 1) noexcept final
    {
    }
    void DrawLine(const FloatPoint &, const FloatPoint &) noexcept final
    {
    }
    void DrawEllipse(const FloatRect &) noexcept final
    {
    }
    void FillPath(const Path &) noexcept final
    {
    }
    void StrokePath(const Path &) noexcept final
    {
    }
    void FillRect(const FloatRect &, RequiresClipToRect) noexcept final
    {
    }
    void FillRect(const FloatRect &, Gradient &, const AffineTransform &, RequiresClipToRect) noexcept final
    {
    }
    void FillRect(const FloatRect &, const Color &) noexcept final
    {
    }
    void FillRoundedRectImpl(const FloatRoundedRect &, const Color &) noexcept final
    {
    }
    void StrokeRect(const FloatRect &, float) noexcept final
    {
    }
    void ClipPath(const Path &, WindRule = WindRule::EvenOdd) noexcept final
    {
    }
    void DrawLinesForText(const FloatPoint &, float, Span<const FloatSegment>, bool, bool,
                          boo::StrokeStyle) noexcept final
    {
    }
    void SetLineCap(LineCap) noexcept final
    {
    }
    void SetLineDash(const DashArray &, float) noexcept final
    {
    }
    void SetLineJoin(LineJoin) noexcept final
    {
    }
    void SetMiterLimit(float) noexcept final
    {
    }
    void ClipOut(const Path &) noexcept final
    {
    }
    void Scale(const FloatSize &) noexcept final
    {
    }
    void Rotate(float) noexcept final
    {
    }
    void Translate(float, float) noexcept final
    {
    }
    void ConcatCTM(const AffineTransform &) noexcept final
    {
    }
    void SetCTM(const AffineTransform &) noexcept final
    {
    }
    KRYS_NODISCARD AffineTransform
      GetCTM(IncludeDeviceScale = PossiblyIncludeDeviceScale) const noexcept final
    {
      return {};
    }
    void ClearRect(const FloatRect &) noexcept final
    {
    }
    void ResetClip() noexcept final
    {
    }
    void Clip(const FloatRect &) noexcept final
    {
    }
    void ClipOut(const FloatRect &) noexcept final
    {
    }
    void Save(GraphicsContextState::Purpose = GraphicsContextState::Purpose::SaveRestore) noexcept final
    {
    }
    void Restore(GraphicsContextState::Purpose = GraphicsContextState::Purpose::SaveRestore) noexcept final
    {
    }
    void DrawRaisedEllipse(const FloatRect &, const Color &, const Color &) noexcept final
    {
    }
    void DrawGlyphs(const Font &, Span<const GlyphBufferGlyph>, Span<const GlyphBufferAdvance>,
                    const FloatPoint &, FontSmoothingMode) noexcept final
    {
    }
    void DrawDisplayList(const display_list::DisplayList &) noexcept final
    {
    }
    void DrawDotsForDocumentMarker(const FloatRect &, DocumentMarkerLineStyle) noexcept final
    {
    }
    void DrawFocusRing(const Path &, float, const Color &) noexcept final
    {
    }
    void DrawFocusRing(const List<FloatRect> &, float, float, const Color &) noexcept final
    {
    }
    void ClipRoundedRect(const FloatRoundedRect &) noexcept final
    {
    }
    void ClipOutRoundedRect(const FloatRoundedRect &) noexcept final
    {
    }
    void FillRect(const FloatRect &, Gradient &) noexcept final
    {
    }
    void FillRect(const FloatRect &, const Color &, CompositeOperator,
                  boo::BlendMode = BlendMode::Normal) noexcept final
    {
    }
    void FillRoundedRect(const FloatRoundedRect &, const Color &,
                         boo::BlendMode = BlendMode::Normal) noexcept final
    {
    }
    void FillRectWithRoundedHole(const FloatRect &, const FloatRoundedRect &, const Color &) noexcept final
    {
    }
    ImageDrawResult DrawImage(Image &, const FloatRect &, const FloatRect &,
                              ImagePaintingOptions = ImagePaintingOptions {
                                ImageOrientation::Orientation::FromImage}) noexcept final
    {
      return ImageDrawResult::DidNothing;
    }
    ImageDrawResult DrawTiledImage(Image &, const FloatRect &, const FloatPoint &, const FloatSize &,
                                   const FloatSize &, ImagePaintingOptions = {}) noexcept final
    {
      return ImageDrawResult::DidNothing;
    }
    void DrawImageBuffer(ImageBuffer &, const FloatRect &, const FloatRect &,
                         ImagePaintingOptions = {}) noexcept final
    {
    }
    void DrawPattern(ImageBuffer &, const FloatRect &, const FloatRect &, const AffineTransform &,
                     const FloatPoint &, const FloatSize &, ImagePaintingOptions = {}) noexcept final
    {
    }
    ImageDrawResult DrawTiledImage(Image &, const FloatRect &, const FloatRect &, const FloatSize &,
                                   Image::TileRule, Image::TileRule, ImagePaintingOptions = {}) noexcept final
    {
      return ImageDrawResult::DidNothing;
    }
  };
}
