/*
 * Copyright (C) 2003-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Graphics/Fonts/GlyphBufferMembers.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSegment.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextState.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/Graphics/Images/ImagePaintingOptions.hpp"
#include "Krystal.Booey/Graphics/Paths/DashArray.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Text/TextFlags.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Strings/StringAtom.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Net/Url.hpp"

namespace krys::boo
{
  class Filter;
  class FilterResults;
  class FloatRoundedRect;
  class Font;
  class FontCascade;
  class Gradient;
  class Path;
  class TextRun;

  enum class RequiresClipToRect : bool
  {
    No,
    Yes,
  };

  namespace display_list
  {
    class DisplayList;
  }

  class ControlPart;
  struct ControlStyle;

  /// @brief Where painting goes: state (brushes, shadows, compositing, transforms, clips) and the drawing
  /// operations, implemented by display list recorders and drawing backends.
  /// FIXME: System images and video frames (`drawConsumingImageBuffer`, `drawSystemImage`, `drawVideoFrame`,
  /// `clipToImageBuffer` and the fill and stroke patterns).
  class GraphicsContext
  {
    KRYS_NON_COPYABLE(GraphicsContext);

  public:
    // Indicates if draw operations read the sources such as NativeImage backing stores immediately
    // during draw operations.
    enum class IsDeferred : bool
    {
      No,
      Yes,
    };

    using RequiresClipToRect = boo::RequiresClipToRect;

    enum IncludeDeviceScale
    {
      DefinitelyIncludeDeviceScale,
      PossiblyIncludeDeviceScale,
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

    GraphicsContext(IsDeferred, GraphicsContextState) noexcept;

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

    KRYS_NODISCARD Pattern *FillPattern() const noexcept
    {
      return FillBrush().GetPattern();
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

    void SetFillPattern(Ref<Pattern> &&pattern) noexcept
    {
      _state.SetFillPattern(krys::move(pattern));
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

    KRYS_NODISCARD Pattern *StrokePattern() const noexcept
    {
      return StrokeBrush().GetPattern();
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

    void SetStrokePattern(Ref<Pattern> &&pattern) noexcept
    {
      _state.SetStrokePattern(krys::move(pattern));
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

    KRYS_NODISCARD static SizeF PlatformShadowOffset(const SizeF &) noexcept;

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
    virtual void DrawRect(const RectF &, float borderThickness = 1) noexcept = 0;

    virtual void DrawLine(const PointF &, const PointF &) noexcept = 0;

    virtual void DrawEllipse(const RectF &) noexcept = 0;

    virtual void DrawRaisedEllipse(const RectF &, const Color &ellipseColor,
                                   const Color &shadowColor) noexcept;

    virtual void FillPath(const Path &) noexcept = 0;

    virtual void StrokePath(const Path &) noexcept = 0;

    virtual void DrawPath(const Path &) noexcept;

    virtual void FillEllipse(const RectF &ellipse) noexcept
    {
      FillEllipseAsPath(ellipse);
    }

    virtual void StrokeEllipse(const RectF &ellipse) noexcept
    {
      StrokeEllipseAsPath(ellipse);
    }

    virtual void FillRect(const RectF &, RequiresClipToRect = RequiresClipToRect::Yes) noexcept = 0;

    virtual void FillRect(const RectF &, const Color &) noexcept = 0;

    virtual void FillRect(const RectF &, Gradient &, const AffineTransform &,
                          RequiresClipToRect = RequiresClipToRect::Yes) noexcept = 0;

    virtual void FillRect(const RectF &, Gradient &) noexcept;

    virtual void FillRect(const RectF &, const Color &, CompositeOperator,
                          boo::BlendMode = BlendMode::Normal) noexcept;

    virtual void FillRoundedRectImpl(const FloatRoundedRect &, const Color &) noexcept = 0;

    virtual void FillRoundedRect(const FloatRoundedRect &, const Color &,
                                 boo::BlendMode = BlendMode::Normal) noexcept;

    virtual void FillRectWithRoundedHole(const RectF &, const FloatRoundedRect &roundedHoleRect,
                                         const Color &) noexcept;

    virtual void ClearRect(const RectF &) noexcept = 0;

    virtual void StrokeRect(const RectF &, float lineWidth) noexcept = 0;

    virtual void SetLineCap(LineCap) noexcept = 0;

    virtual void SetLineDash(const DashArray &, float dashOffset) noexcept = 0;

    virtual void SetLineJoin(LineJoin) noexcept = 0;

    virtual void SetMiterLimit(float) noexcept = 0;

    // Images

    ImageDrawResult DrawImage(Image &, const PointF &destination,
                              ImagePaintingOptions = ImagePaintingOptions {
                                ImageOrientation::Orientation::FromImage}) noexcept;

    ImageDrawResult DrawImage(Image &, const RectF &destination,
                              ImagePaintingOptions = ImagePaintingOptions {
                                ImageOrientation::Orientation::FromImage}) noexcept;

    virtual ImageDrawResult DrawImage(Image &, const RectF &destination, const RectF &source,
                                      ImagePaintingOptions = ImagePaintingOptions {
                                        ImageOrientation::Orientation::FromImage}) noexcept;

    virtual ImageDrawResult DrawTiledImage(Image &, const RectF &destination, const PointF &source,
                                           const SizeF &tileSize, const SizeF &spacing,
                                           ImagePaintingOptions = {}) noexcept;

    // Native images

    /// @brief Draws @p srcRect of the image into @p destRect, oriented by the options' orientation.
    virtual void DrawNativeImage(NativeImage &, const RectF &destRect, const RectF &srcRect,
                                 ImagePaintingOptions = {}) noexcept = 0;

    /// @brief Fills @p destRect with @p tileRect of the image, repeated: tiles are mapped by @p
    /// patternTransform, start at @p phase (where the image's origin goes) and are @p spacing apart.
    virtual void DrawPattern(NativeImage &, const RectF &destRect, const RectF &tileRect,
                             const AffineTransform &patternTransform, const PointF &phase,
                             const SizeF &spacing, ImagePaintingOptions = {}) noexcept = 0;

    // Image buffers

    KRYS_NODISCARD virtual RefPtr<ImageBuffer> CreateImageBuffer(const SizeF &,
                                                                 float resolutionScale = 1) const noexcept;

    KRYS_NODISCARD RefPtr<ImageBuffer> CreateScaledImageBuffer(const SizeF &,
                                                               const SizeF &scale) const noexcept;

    KRYS_NODISCARD RefPtr<ImageBuffer> CreateScaledImageBuffer(const RectF &,
                                                               const SizeF &scale) const noexcept;

    /// @brief A buffer of @p size at the resolution this context draws at, its context scaled so @p size
    /// fills it.
    KRYS_NODISCARD virtual RefPtr<ImageBuffer> CreateAlignedImageBuffer(const SizeF &size) const noexcept;

    void DrawImageBuffer(ImageBuffer &, const PointF &destination, ImagePaintingOptions = {}) noexcept;

    void DrawImageBuffer(ImageBuffer &, const RectF &destination, ImagePaintingOptions = {}) noexcept;

    /// @brief Draws @p source of the buffer into @p destination.
    /// FIXME: Buffers record display lists rather than pixels, so the base draws nothing (rather than the
    /// buffer's native image); display list recorders record the draw and drawing backends render the buffer.
    virtual void DrawImageBuffer(ImageBuffer &, const RectF &destination, const RectF &source,
                                 ImagePaintingOptions = {}) noexcept;

    /// @brief Draws @p filter applied to @p sourceImage, which holds @p sourceImageRect (in filter
    /// coordinates), with @p results caching the filter's effects.
    virtual void DrawFilteredImageBuffer(ImageBuffer *sourceImage, const RectF &sourceImageRect,
                                         Filter &filter, FilterResults &results) noexcept;

    /// @brief Fills @p destRect with @p tileRect of the buffer, repeated: tiles are mapped by @p
    /// patternTransform, start at @p phase (where the buffer's origin goes) and are @p spacing apart.
    /// FIXME: Buffers record display lists rather than pixels, so the base draws nothing (rather than the
    /// buffer's native image); display list recorders record the draw and drawing backends render the buffer.
    virtual void DrawPattern(ImageBuffer &, const RectF &destRect, const RectF &tileRect,
                             const AffineTransform &patternTransform, const PointF &phase,
                             const SizeF &spacing, ImagePaintingOptions = {}) noexcept;

    virtual ImageDrawResult DrawTiledImage(Image &, const RectF &destination, const RectF &source,
                                           const SizeF &tileScaleFactor, Image::TileRule, Image::TileRule,
                                           ImagePaintingOptions = {}) noexcept;

    // Clipping

    virtual void ResetClip() noexcept = 0;

    virtual void Clip(const RectF &) noexcept = 0;

    virtual void ClipRoundedRect(const FloatRoundedRect &) noexcept;

    virtual void ClipOut(const RectF &) noexcept = 0;

    virtual void ClipOut(const Path &) noexcept = 0;

    virtual void ClipOutRoundedRect(const FloatRoundedRect &) noexcept;

    virtual void ClipPath(const Path &, WindRule = WindRule::EvenOdd) noexcept = 0;

    KRYS_NODISCARD virtual RectI ClipBounds() const noexcept;

    virtual void ClipToImageBuffer(ImageBuffer &, const RectF &) noexcept = 0;

    // Text

    /// @brief Draws @p run from byte offset @p from to @p to at @p point, through `DrawGlyphs`.
    /// @return How far the first glyph drawn is from @p point.
    virtual SizeF DrawText(const FontCascade &, const TextRun &, const PointF &, uint32 from = 0,
                           Maybe<uint32> to = null) noexcept;

    /// @brief Draws @p run at @p point, split into runs of one direction by the bidi algorithm (from the
    /// run's direction and override) and drawn in visual order.
    virtual void DrawBidiText(const FontCascade &, const TextRun &, const PointF &,
                              FontCascade::CustomFontNotReadyAction =
                                FontCascade::CustomFontNotReadyAction::DoNotPaintIfFontNotReady) noexcept;

    virtual void DrawEmphasisMarks(const FontCascade &, const TextRun &, const StringAtom &mark,
                                   const PointF &, uint32 from = 0, Maybe<uint32> to = null) noexcept;

    virtual void DrawGlyphs(const Font &, Span<const GlyphBufferGlyph>, Span<const GlyphBufferAdvance>,
                            const PointF &, FontSmoothingMode) noexcept = 0;

    virtual void DrawDisplayList(const display_list::DisplayList &) noexcept;

    /// @brief Draws a form control as the theme does.
    virtual void DrawControlPart(ControlPart &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
                                 const ControlStyle &) noexcept;

    KRYS_NODISCARD RectF ComputeUnderlineBoundsForText(const RectF &, bool printing) noexcept;

    void DrawLineForText(const RectF &, bool isPrinting, bool doubleUnderlines = false,
                         boo::StrokeStyle = boo::StrokeStyle::SolidStroke) noexcept;

    // The `origin` defines the line origin point.
    // The `lineSegments` defines the start and end offset of each segment along the line.
    virtual void DrawLinesForText(const PointF &origin, float thickness,
                                  Span<const FloatSegment> lineSegments, bool isPrinting, bool doubleLines,
                                  boo::StrokeStyle) noexcept = 0;

    virtual void DrawDotsForDocumentMarker(const RectF &, DocumentMarkerLineStyle) noexcept = 0;

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

    virtual void DrawFocusRing(const List<RectF> &, float outlineOffset, float outlineWidth,
                               const Color &) noexcept = 0;

    // Transforms

    void Scale(float s) noexcept
    {
      Scale(SizeF {s, s});
    }

    virtual void Scale(const SizeF &) noexcept = 0;

    virtual void Rotate(float angleInRadians) noexcept = 0;

    void Translate(const SizeF &size) noexcept
    {
      Translate(size.Width(), size.Height());
    }

    void Translate(const PointF &p) noexcept
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

    KRYS_NODISCARD SizeF ScaleFactor() const noexcept;

    KRYS_NODISCARD SizeF ScaleFactorForDrawing(const RectF &destRect, const RectF &srcRect) const noexcept;

    // PDF, printing and snapshotting
    virtual void BeginPage(const RectF &) noexcept
    {
    }

    virtual void EndPage() noexcept
    {
    }

    // Links

    virtual void SetUrlForRect(const net::Url &, const RectF &) noexcept
    {
    }

    virtual void SetDestinationForRect(const utf8_string &, const RectF &) noexcept
    {
    }

    virtual void AddDestinationAtPoint(const utf8_string &, const PointF &) noexcept
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

    /// @brief A dash and the gap after it.
    struct DashPattern
    {
      float Dash {0};
      float Gap {0};
    };

    /// @brief The dashes of a dashed line or border, as Chrome has them: each twice the thickness and a gap
    /// of the thickness (three and two times when it's under 3), with the gap then stretched or squeezed so
    /// that dashes meet both ends of the line.
    /// @param length The length of the stroke.
    /// @param thickness The thickness of the line.
    /// @param closedPath Whether the stroke goes back to its start, with a gap there instead of an end.
    /// @return The dashes, or null if there's no room for two: the line is then solid.
    KRYS_NODISCARD static Maybe<DashPattern> DashPatternForStrokeLength(float length, float thickness,
                                                                        bool closedPath) noexcept;

  protected:
    void FillEllipseAsPath(const RectF &) noexcept;

    void StrokeEllipseAsPath(const RectF &) noexcept;

    virtual void DrawGlyphsImmediate(const Font &, Span<const GlyphBufferGlyph>,
                                     Span<const GlyphBufferAdvance>, const PointF &,
                                     FontSmoothingMode) noexcept;

    KRYS_NODISCARD RectF ComputeLineBoundsAndAntialiasingModeForText(const RectF &, bool printing,
                                                                     Color &) const noexcept;

    KRYS_NODISCARD float DashedLineCornerWidthForStrokeWidth(float) const noexcept;

    KRYS_NODISCARD float DashedLinePatternWidthForStrokeWidth(float) const noexcept;

    KRYS_NODISCARD static float DashedLinePatternOffsetForPatternAndStrokeWidth(float patternWidth,
                                                                                float strokeWidth) noexcept;

    KRYS_NODISCARD static List<PointF> CenterLineAndCutOffCorners(bool isVerticalLine, float cornerWidth,
                                                                  PointF point1, PointF point2) noexcept;

    struct RectsAndStrokeColor
    {
      SmallList<RectF, 4> Rects;
      Color StrokeColor;
    };

    KRYS_NODISCARD RectsAndStrokeColor ComputeRectsAndStrokeColorForLinesForText(
      const PointF &origin, float thickness, Span<const FloatSegment>, bool isPrinting, bool doubleLines,
      boo::StrokeStyle) noexcept;
  };
}

#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
