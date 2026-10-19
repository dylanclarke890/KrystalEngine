#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Graphics/DashArray.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSegment.hpp"
#include "Krystal.Booey/Graphics/GlyphBufferMembers.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextState.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ImagePaintingOptions.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class FloatRoundedRect;
  class Font;
  class FontCascade;
  class Gradient;
  class Path;
  class TextRun;

  enum class RequiresClipToRect : bool
  {
    No,
    Yes
  };

  namespace display_list
  {
    class DisplayList;
  }

  /// @brief Where painting goes: state (brushes, shadows, compositing, transforms, clips) and the drawing
  /// operations, implemented by display list recorders and drawing backends.
  /// FIXME: Bitmap images, filters, system images, video frames and form controls (`drawNativeImage`,
  /// `drawConsumingImageBuffer`, `drawFilteredImageBuffer`, `drawPattern` of native images,
  /// `drawSystemImage`, `drawVideoFrame`, `drawControlPart`, `clipToImageBuffer` and the fill and stroke
  /// patterns) come with images.
  /// FIXME: Bidirectional text runs (`drawBidiText`) need a bidi resolver over text runs.
  class GraphicsContext
  {
    KRYS_NON_COPYABLE(GraphicsContext);

  public:
    // Indicates if draw operations read the sources such as NativeImage backing stores immediately
    // during draw operations.
    enum class IsDeferred : bool
    {
      No,
      Yes
    };

    using RequiresClipToRect = boo::RequiresClipToRect;

    enum IncludeDeviceScale
    {
      DefinitelyIncludeDeviceScale,
      PossiblyIncludeDeviceScale
    };

  protected:
    GraphicsContextState _state;

  private:
    SmallList<GraphicsContextState, 1> _stack;

    uint32 _transparencyLayerCount {0};
    const IsDeferred _isDeferred;
    bool _contentfulPaintDetected {false};

  public:
    GraphicsContext(IsDeferred = IsDeferred::No, const GraphicsContextState::ChangeFlags & = {},
                    InterpolationQuality = InterpolationQuality::Default) noexcept;
    GraphicsContext(IsDeferred, const GraphicsContextState &) noexcept;
    virtual ~GraphicsContext() noexcept;

    KRYS_NODISCARD virtual bool PaintingDisabled() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool PerformingPaintInvalidation() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool InvalidatingControlTints() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool InvalidatingImagesWithAsyncDecodes() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool DetectingContentfulPaint() const noexcept
    {
      return false;
    }

    // Context State

    KRYS_NODISCARD const SourceBrush &FillBrush() const noexcept
    {
      return _state.FillBrush();
    }
    KRYS_NODISCARD const Color &FillColor() const noexcept
    {
      return FillBrush().GetColor();
    }
    KRYS_NODISCARD Gradient *FillGradient() const noexcept
    {
      return FillBrush().GetGradient();
    }
    KRYS_NODISCARD const AffineTransform &FillGradientSpaceTransform() const noexcept
    {
      return FillBrush().GradientSpaceTransform();
    }
    void SetFillBrush(const SourceBrush &brush) noexcept
    {
      _state.SetFillBrush(brush);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::FillBrush));
    }
    void SetFillColor(const Color &color) noexcept
    {
      _state.SetFillColor(color);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::FillBrush));
    }
    void SetFillGradient(Ref<Gradient> &&gradient, const AffineTransform &spaceTransform = {}) noexcept
    {
      _state.SetFillGradient(krys::move(gradient), spaceTransform);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::FillBrush));
    }

    KRYS_NODISCARD WindRule FillRule() const noexcept
    {
      return _state.FillRule();
    }
    void SetFillRule(WindRule fillRule) noexcept
    {
      _state.SetFillRule(fillRule);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::FillRule));
    }

    KRYS_NODISCARD const SourceBrush &StrokeBrush() const noexcept
    {
      return _state.StrokeBrush();
    }
    KRYS_NODISCARD const Color &StrokeColor() const noexcept
    {
      return StrokeBrush().GetColor();
    }
    KRYS_NODISCARD Gradient *StrokeGradient() const noexcept
    {
      return StrokeBrush().GetGradient();
    }
    KRYS_NODISCARD const AffineTransform &StrokeGradientSpaceTransform() const noexcept
    {
      return StrokeBrush().GradientSpaceTransform();
    }
    void SetStrokeBrush(const SourceBrush &brush) noexcept
    {
      _state.SetStrokeBrush(brush);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::StrokeBrush));
    }
    void SetStrokeColor(const Color &color) noexcept
    {
      _state.SetStrokeColor(color);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::StrokeBrush));
    }
    void SetStrokeGradient(Ref<Gradient> &&gradient, const AffineTransform &spaceTransform = {}) noexcept
    {
      _state.SetStrokeGradient(krys::move(gradient), spaceTransform);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::StrokeBrush));
    }

    KRYS_NODISCARD float StrokeThickness() const noexcept
    {
      return _state.StrokeThickness();
    }
    void SetStrokeThickness(float thickness) noexcept
    {
      _state.SetStrokeThickness(thickness);
      DidUpdateSingleState(_state,
                           GraphicsContextState::ToIndex(GraphicsContextState::Change::StrokeThickness));
    }

    KRYS_NODISCARD boo::StrokeStyle GetStrokeStyle() const noexcept
    {
      return _state.GetStrokeStyle();
    }
    void SetStrokeStyle(boo::StrokeStyle style) noexcept
    {
      _state.SetStrokeStyle(style);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::StrokeStyle));
    }

    KRYS_NODISCARD Maybe<GraphicsDropShadow> DropShadow() const noexcept
    {
      return _state.DropShadow();
    }
    void SetDropShadow(const GraphicsDropShadow &dropShadow) noexcept
    {
      _state.SetDropShadow(dropShadow);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::DropShadow));
    }
    void ClearDropShadow() noexcept
    {
      _state.SetDropShadow(std::nullopt);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::DropShadow));
    }
    KRYS_NODISCARD bool HasBlurredDropShadow() const noexcept
    {
      return DropShadow() && DropShadow()->IsBlurred();
    }
    KRYS_NODISCARD bool HasDropShadow() const noexcept
    {
      return DropShadow() && DropShadow()->HasOutsets();
    }

    KRYS_NODISCARD Maybe<GraphicsStyle> Style() const noexcept
    {
      return _state.Style();
    }
    void SetStyle(const Maybe<GraphicsStyle> &style) noexcept
    {
      _state.SetStyle(style);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::Style));
    }

    KRYS_NODISCARD boo::CompositeMode GetCompositeMode() const noexcept
    {
      return _state.GetCompositeMode();
    }
    KRYS_NODISCARD CompositeOperator CompositeOperation() const noexcept
    {
      return GetCompositeMode().Operation;
    }
    KRYS_NODISCARD boo::BlendMode GetBlendMode() const noexcept
    {
      return GetCompositeMode().BlendMode;
    }
    void SetCompositeMode(boo::CompositeMode compositeMode) noexcept
    {
      _state.SetCompositeMode(compositeMode);
      DidUpdateSingleState(_state,
                           GraphicsContextState::ToIndex(GraphicsContextState::Change::CompositeMode));
    }
    void SetCompositeOperation(CompositeOperator operation,
                               boo::BlendMode blendMode = BlendMode::Normal) noexcept
    {
      SetCompositeMode({operation, blendMode});
    }

    KRYS_NODISCARD float Alpha() const noexcept
    {
      return _state.Alpha();
    }
    void SetAlpha(float alpha) noexcept
    {
      _state.SetAlpha(alpha);
      DidUpdateSingleState(_state, GraphicsContextState::ToIndex(GraphicsContextState::Change::Alpha));
    }

    KRYS_NODISCARD TextDrawingModeFlags GetTextDrawingMode() const noexcept
    {
      return _state.GetTextDrawingMode();
    }
    void SetTextDrawingMode(TextDrawingModeFlags textDrawingMode) noexcept
    {
      _state.SetTextDrawingMode(textDrawingMode);
      DidUpdateSingleState(_state,
                           GraphicsContextState::ToIndex(GraphicsContextState::Change::TextDrawingMode));
    }

    KRYS_NODISCARD InterpolationQuality ImageInterpolationQuality() const noexcept
    {
      return _state.ImageInterpolationQuality();
    }
    void SetImageInterpolationQuality(InterpolationQuality imageInterpolationQuality) noexcept
    {
      _state.SetImageInterpolationQuality(imageInterpolationQuality);
      DidUpdateSingleState(
        _state, GraphicsContextState::ToIndex(GraphicsContextState::Change::ImageInterpolationQuality));
    }

    KRYS_NODISCARD bool ShouldAntialias() const noexcept
    {
      return _state.ShouldAntialias();
    }
    void SetShouldAntialias(bool shouldAntialias) noexcept
    {
      _state.SetShouldAntialias(shouldAntialias);
      DidUpdateSingleState(_state,
                           GraphicsContextState::ToIndex(GraphicsContextState::Change::ShouldAntialias));
    }

    KRYS_NODISCARD bool ShouldSmoothFonts() const noexcept
    {
      return _state.ShouldSmoothFonts();
    }
    void SetShouldSmoothFonts(bool shouldSmoothFonts) noexcept
    {
      _state.SetShouldSmoothFonts(shouldSmoothFonts);
      DidUpdateSingleState(_state,
                           GraphicsContextState::ToIndex(GraphicsContextState::Change::ShouldSmoothFonts));
    }

    // Normally CG enables subpixel-quantization because it improves the performance of aligning glyphs.
    // In some cases we have to disable to to ensure a high-quality output of the glyphs.
    KRYS_NODISCARD bool ShouldSubpixelQuantizeFonts() const noexcept
    {
      return _state.ShouldSubpixelQuantizeFonts();
    }
    void SetShouldSubpixelQuantizeFonts(bool shouldSubpixelQuantizeFonts) noexcept
    {
      _state.SetShouldSubpixelQuantizeFonts(shouldSubpixelQuantizeFonts);
      DidUpdateSingleState(
        _state, GraphicsContextState::ToIndex(GraphicsContextState::Change::ShouldSubpixelQuantizeFonts));
    }

    KRYS_NODISCARD bool ShadowsIgnoreTransforms() const noexcept
    {
      return _state.ShadowsIgnoreTransforms();
    }
    void SetShadowsIgnoreTransforms(bool shadowsIgnoreTransforms) noexcept
    {
      _state.SetShadowsIgnoreTransforms(shadowsIgnoreTransforms);
      DidUpdateSingleState(
        _state, GraphicsContextState::ToIndex(GraphicsContextState::Change::ShadowsIgnoreTransforms));
    }
    KRYS_NODISCARD FloatSize PlatformShadowOffset(const FloatSize &) const noexcept;

    KRYS_NODISCARD bool DrawLuminanceMask() const noexcept
    {
      return _state.DrawLuminanceMask();
    }
    void SetDrawLuminanceMask(bool drawLuminanceMask) noexcept
    {
      _state.SetDrawLuminanceMask(drawLuminanceMask);
      DidUpdateSingleState(_state,
                           GraphicsContextState::ToIndex(GraphicsContextState::Change::DrawLuminanceMask));
    }

    KRYS_NODISCARD virtual const GraphicsContextState &State() const noexcept
    {
      return _state;
    }
    void MergeLastChanges(const GraphicsContextState &,
                          const Maybe<GraphicsContextState> &lastDrawingState = std::nullopt) noexcept;
    void MergeAllChanges(const GraphicsContextState &) noexcept;

    // Called *after* any change to GraphicsContextState; generally used to propagate changes
    // to the platform context's state.
    virtual void DidUpdateState(GraphicsContextState &) noexcept = 0;
    virtual void DidUpdateSingleState(GraphicsContextState &state, GraphicsContextState::ChangeIndex) noexcept
    {
      DidUpdateState(state);
    }

    virtual void Save(GraphicsContextState::Purpose = GraphicsContextState::Purpose::SaveRestore) noexcept;
    virtual void Restore(GraphicsContextState::Purpose = GraphicsContextState::Purpose::SaveRestore) noexcept;

    void UnwindStateStack(uint32 count) noexcept;
    void UnwindStateStack() noexcept;

    KRYS_NODISCARD uint32 StackSize() const noexcept
    {
      return static_cast<uint32>(_stack.size());
    }

    KRYS_NODISCARD virtual bool KnownToHaveFloatBasedBacking() const noexcept
    {
      return false;
    }

    // Shapes

    // These draw methods will do both stroking and filling.
    // FIXME: ...except drawRect(), which fills properly but always strokes
    // using a 1-pixel stroke inset from the rect borders (of the correct
    // stroke color).
    virtual void DrawRect(const FloatRect &, float borderThickness = 1) noexcept = 0;
    virtual void DrawLine(const FloatPoint &, const FloatPoint &) noexcept = 0;

    virtual void DrawEllipse(const FloatRect &) noexcept = 0;
    virtual void DrawRaisedEllipse(const FloatRect &, const Color &ellipseColor,
                                   const Color &shadowColor) noexcept;

    virtual void FillPath(const Path &) noexcept = 0;
    virtual void StrokePath(const Path &) noexcept = 0;
    virtual void DrawPath(const Path &) noexcept;

    virtual void FillEllipse(const FloatRect &ellipse) noexcept
    {
      FillEllipseAsPath(ellipse);
    }
    virtual void StrokeEllipse(const FloatRect &ellipse) noexcept
    {
      StrokeEllipseAsPath(ellipse);
    }

    virtual void FillRect(const FloatRect &, RequiresClipToRect = RequiresClipToRect::Yes) noexcept = 0;
    virtual void FillRect(const FloatRect &, const Color &) noexcept = 0;
    virtual void FillRect(const FloatRect &, Gradient &, const AffineTransform &,
                          RequiresClipToRect = RequiresClipToRect::Yes) noexcept = 0;
    virtual void FillRect(const FloatRect &, Gradient &) noexcept;
    virtual void FillRect(const FloatRect &, const Color &, CompositeOperator,
                          boo::BlendMode = BlendMode::Normal) noexcept;
    virtual void FillRoundedRectImpl(const FloatRoundedRect &, const Color &) noexcept = 0;
    virtual void FillRoundedRect(const FloatRoundedRect &, const Color &,
                                 boo::BlendMode = BlendMode::Normal) noexcept;
    virtual void FillRectWithRoundedHole(const FloatRect &, const FloatRoundedRect &roundedHoleRect,
                                         const Color &) noexcept;

    virtual void ClearRect(const FloatRect &) noexcept = 0;

    virtual void StrokeRect(const FloatRect &, float lineWidth) noexcept = 0;

    virtual void SetLineCap(LineCap) noexcept = 0;
    virtual void SetLineDash(const DashArray &, float dashOffset) noexcept = 0;
    virtual void SetLineJoin(LineJoin) noexcept = 0;
    virtual void SetMiterLimit(float) noexcept = 0;

    // Images

    ImageDrawResult DrawImage(Image &, const FloatPoint &destination,
                              ImagePaintingOptions = ImagePaintingOptions {
                                ImageOrientation::Orientation::FromImage}) noexcept;
    ImageDrawResult DrawImage(Image &, const FloatRect &destination,
                              ImagePaintingOptions = ImagePaintingOptions {
                                ImageOrientation::Orientation::FromImage}) noexcept;
    virtual ImageDrawResult DrawImage(Image &, const FloatRect &destination, const FloatRect &source,
                                      ImagePaintingOptions = ImagePaintingOptions {
                                        ImageOrientation::Orientation::FromImage}) noexcept;

    virtual ImageDrawResult DrawTiledImage(Image &, const FloatRect &destination, const FloatPoint &source,
                                           const FloatSize &tileSize, const FloatSize &spacing,
                                           ImagePaintingOptions = {}) noexcept;

    // Image buffers

    KRYS_NODISCARD virtual RefPtr<ImageBuffer> CreateImageBuffer(const FloatSize &,
                                                                 float resolutionScale = 1) const noexcept;
    KRYS_NODISCARD RefPtr<ImageBuffer> CreateScaledImageBuffer(const FloatSize &,
                                                               const FloatSize &scale) const noexcept;
    /// @brief A buffer of @p size at the resolution this context draws at, its context scaled so @p size
    /// fills it.
    KRYS_NODISCARD virtual RefPtr<ImageBuffer> CreateAlignedImageBuffer(const FloatSize &size) const noexcept;

    void DrawImageBuffer(ImageBuffer &, const FloatPoint &destination, ImagePaintingOptions = {}) noexcept;
    void DrawImageBuffer(ImageBuffer &, const FloatRect &destination, ImagePaintingOptions = {}) noexcept;
    /// @brief Draws @p source of the buffer into @p destination.
    /// FIXME: Without native images, the base draws nothing; display list recorders record the draw and
    /// drawing backends render the buffer.
    virtual void DrawImageBuffer(ImageBuffer &, const FloatRect &destination, const FloatRect &source,
                                 ImagePaintingOptions = {}) noexcept;

    /// @brief Fills @p destRect with @p tileRect of the buffer, repeated: tiles are mapped by @p
    /// patternTransform, start at @p phase (where the buffer's origin goes) and are @p spacing apart.
    /// FIXME: Without native images, the base draws nothing; display list recorders record the draw and
    /// drawing backends render the buffer.
    virtual void DrawPattern(ImageBuffer &, const FloatRect &destRect, const FloatRect &tileRect,
                             const AffineTransform &patternTransform, const FloatPoint &phase,
                             const FloatSize &spacing, ImagePaintingOptions = {}) noexcept;
    virtual ImageDrawResult DrawTiledImage(Image &, const FloatRect &destination, const FloatRect &source,
                                           const FloatSize &tileScaleFactor, Image::TileRule, Image::TileRule,
                                           ImagePaintingOptions = {}) noexcept;

    // Clipping

    virtual void ResetClip() noexcept = 0;
    virtual void Clip(const FloatRect &) noexcept = 0;
    virtual void ClipRoundedRect(const FloatRoundedRect &) noexcept;

    virtual void ClipOut(const FloatRect &) noexcept = 0;
    virtual void ClipOut(const Path &) noexcept = 0;
    virtual void ClipOutRoundedRect(const FloatRoundedRect &) noexcept;
    virtual void ClipPath(const Path &, WindRule = WindRule::EvenOdd) noexcept = 0;
    KRYS_NODISCARD virtual IntRect ClipBounds() const noexcept;

    // Text

    /// @brief Draws @p run from byte offset @p from to @p to at @p point, through `DrawGlyphs`.
    /// @return How far the first glyph drawn is from @p point.
    virtual FloatSize DrawText(const FontCascade &, const TextRun &, const FloatPoint &, uint32 from = 0,
                               Maybe<uint32> to = null) noexcept;
    virtual void DrawEmphasisMarks(const FontCascade &, const TextRun &, const StringAtom &mark,
                                   const FloatPoint &, uint32 from = 0, Maybe<uint32> to = null) noexcept;
    virtual void DrawGlyphs(const Font &, Span<const GlyphBufferGlyph>, Span<const GlyphBufferAdvance>,
                            const FloatPoint &, FontSmoothingMode) noexcept = 0;
    virtual void DrawDisplayList(const display_list::DisplayList &) noexcept;
    KRYS_NODISCARD FloatRect ComputeUnderlineBoundsForText(const FloatRect &, bool printing) noexcept;
    void DrawLineForText(const FloatRect &, bool isPrinting, bool doubleLines = false,
                         boo::StrokeStyle = boo::StrokeStyle::SolidStroke) noexcept;
    // The `origin` defines the line origin point.
    // The `lineSegments` defines the start and end offset of each segment along the line.
    virtual void DrawLinesForText(const FloatPoint &origin, float thickness,
                                  Span<const FloatSegment> lineSegments, bool isPrinting, bool doubleLines,
                                  boo::StrokeStyle) noexcept = 0;
    virtual void DrawDotsForDocumentMarker(const FloatRect &, DocumentMarkerLineStyle) noexcept = 0;

    // Transparency Layers

    virtual void BeginTransparencyLayer(float opacity) noexcept;
    virtual void BeginTransparencyLayer(CompositeOperator, boo::BlendMode = BlendMode::Normal) noexcept;
    virtual void EndTransparencyLayer() noexcept;
    KRYS_NODISCARD bool IsInTransparencyLayer() const noexcept
    {
      return _transparencyLayerCount > 0;
    }

    // Focus Rings

    virtual void DrawFocusRing(const Path &, float outlineWidth, const Color &) noexcept = 0;
    virtual void DrawFocusRing(const List<FloatRect> &, float outlineOffset, float outlineWidth,
                               const Color &) noexcept = 0;

    // Transforms

    void Scale(float s) noexcept
    {
      Scale(FloatSize {s, s});
    }
    virtual void Scale(const FloatSize &) noexcept = 0;
    virtual void Rotate(float angleInRadians) noexcept = 0;
    void Translate(const FloatSize &size) noexcept
    {
      Translate(size.Width(), size.Height());
    }
    void Translate(const FloatPoint &p) noexcept
    {
      Translate(p.X(), p.Y());
    }
    virtual void Translate(float x, float y) noexcept = 0;

    virtual void ConcatCTM(const AffineTransform &) noexcept = 0;
    virtual void SetCTM(const AffineTransform &) noexcept = 0;

    KRYS_NODISCARD virtual AffineTransform
      GetCTM(IncludeDeviceScale = PossiblyIncludeDeviceScale) const noexcept = 0;

    // This function applies the device scale factor to the context, making the context capable of
    // acting as a base-level context for a HiDPI environment.
    virtual void ApplyDeviceScaleFactor(float factor) noexcept
    {
      Scale(factor);
    }
    KRYS_NODISCARD FloatSize ScaleFactor() const noexcept;
    KRYS_NODISCARD FloatSize ScaleFactorForDrawing(const FloatRect &destRect,
                                                   const FloatRect &srcRect) const noexcept;

    // PDF, printing and snapshotting
    virtual void BeginPage(const FloatRect &) noexcept
    {
    }
    virtual void EndPage() noexcept
    {
    }

    // Links

    virtual void SetURLForRect(const url::URL &, const FloatRect &) noexcept
    {
    }

    virtual void SetDestinationForRect(const utf8_string &, const FloatRect &) noexcept
    {
    }
    virtual void AddDestinationAtPoint(const utf8_string &, const FloatPoint &) noexcept
    {
    }

    KRYS_NODISCARD virtual bool SupportsInternalLinks() const noexcept
    {
      return false;
    }

    // Contentful Paint Detection

    void SetContentfulPaintDetected() noexcept
    {
      _contentfulPaintDetected = true;
    }
    KRYS_NODISCARD bool ContentfulPaintDetected() const noexcept
    {
      return _contentfulPaintDetected;
    }

  protected:
    void FillEllipseAsPath(const FloatRect &) noexcept;
    void StrokeEllipseAsPath(const FloatRect &) noexcept;
    virtual void DrawGlyphsImmediate(const Font &, Span<const GlyphBufferGlyph>,
                                     Span<const GlyphBufferAdvance>, const FloatPoint &,
                                     FontSmoothingMode) noexcept;

    KRYS_NODISCARD FloatRect ComputeLineBoundsAndAntialiasingModeForText(const FloatRect &, bool printing,
                                                                         Color &) noexcept;

    KRYS_NODISCARD float DashedLineCornerWidthForStrokeWidth(float) const noexcept;
    KRYS_NODISCARD float DashedLinePatternWidthForStrokeWidth(float) const noexcept;
    KRYS_NODISCARD float DashedLinePatternOffsetForPatternAndStrokeWidth(float patternWidth,
                                                                         float strokeWidth) const noexcept;
    KRYS_NODISCARD List<FloatPoint> CenterLineAndCutOffCorners(bool isVerticalLine, float cornerWidth,
                                                               FloatPoint point1,
                                                               FloatPoint point2) const noexcept;

    struct RectsAndStrokeColor
    {
      SmallList<FloatRect, 4> Rects;
      Color StrokeColor;
    };
    KRYS_NODISCARD RectsAndStrokeColor ComputeRectsAndStrokeColorForLinesForText(
      const FloatPoint &origin, float thickness, Span<const FloatSegment>, bool isPrinting, bool doubleLines,
      boo::StrokeStyle) noexcept;
  };
}

#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
