/*
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Graphics/GraphicsContext.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Text/Bidi.hpp"
#include "Krystal.Web/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Web/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Web/Graphics/Filters/Filter.hpp"
#include "Krystal.Web/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Web/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Web/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Web/Graphics/Gradient.hpp"
#include "Krystal.Web/Graphics/Images/Image.hpp"
#include "Krystal.Web/Graphics/Paths/Path.hpp"
#include <cmath>

namespace krys::web
{
  GraphicsContext::GraphicsContext(IsDeferred isDeferred,
                                   const GraphicsContextState::ChangeFlags &changeFlags,
                                   InterpolationQuality imageInterpolationQuality) noexcept
      : _state(changeFlags, imageInterpolationQuality), _isDeferred(isDeferred)
  {
  }

  GraphicsContext::GraphicsContext(IsDeferred isDeferred, GraphicsContextState state) noexcept
      : _state(krys::move(state)), _isDeferred(isDeferred)
  {
  }

  GraphicsContext::~GraphicsContext() noexcept
  {
    krys_debug_assert(_stack.empty());
    krys_debug_assert(!_transparencyLayerCount);
  }

  void GraphicsContext::Save(GraphicsContextState::Purpose purpose) noexcept
  {
    krys_debug_assert(purpose == GraphicsContextState::Purpose::SaveRestore
                      || purpose == GraphicsContextState::Purpose::TransparencyLayer);
    _stack.push_back(_state);
    _state.Repurpose(purpose);
  }

  void GraphicsContext::Restore(KRYS_MAYBE_UNUSED GraphicsContextState::Purpose purpose) noexcept
  {
    if (_stack.empty())
    {
      return;
    }

    krys_debug_assert(purpose == _state.GetPurpose());
    krys_debug_assert(purpose == GraphicsContextState::Purpose::SaveRestore
                      || purpose == GraphicsContextState::Purpose::TransparencyLayer);

    _state = _stack.back();
    _stack.pop_back();
  }

  void GraphicsContext::UnwindStateStack(uint32 count) noexcept
  {
    krys_debug_assert(count <= StackSize());
    while (count-- > 0)
    {
      switch (_state.GetPurpose())
      {
        case GraphicsContextState::Purpose::SaveRestore:       Restore(); break;
        case GraphicsContextState::Purpose::TransparencyLayer: EndTransparencyLayer(); break;
        default:                                               krys_debug_assert(false); break;
      }
    }
  }

  void GraphicsContext::UnwindStateStack() noexcept
  {
    UnwindStateStack(StackSize());
  }

  SizeF GraphicsContext::PlatformShadowOffset(const SizeF &shadowOffset) noexcept
  {
    return shadowOffset;
  }

  void GraphicsContext::MergeLastChanges(const GraphicsContextState &state,
                                         const Maybe<GraphicsContextState> &lastDrawingState) noexcept
  {
    _state.MergeLastChanges(state, lastDrawingState);
    DidUpdateState(_state);
  }

  void GraphicsContext::MergeAllChanges(const GraphicsContextState &state) noexcept
  {
    _state.MergeAllChanges(state);
    DidUpdateState(_state);
  }

  void GraphicsContext::DrawRaisedEllipse(const RectF &rect, const Color &ellipseColor,
                                          const Color &shadowColor) noexcept
  {
    Save();

    SetStrokeColor(shadowColor);
    SetFillColor(shadowColor);

    DrawEllipse(RectF(rect.X(), rect.Y() + 1, rect.Width(), rect.Height()));

    SetStrokeColor(ellipseColor);
    SetFillColor(ellipseColor);

    DrawEllipse(rect);

    Restore();
  }

  void GraphicsContext::BeginTransparencyLayer(float) noexcept
  {
    ++_transparencyLayerCount;
  }

  void GraphicsContext::BeginTransparencyLayer(CompositeOperator, web::BlendMode) noexcept
  {
    ++_transparencyLayerCount;
  }

  void GraphicsContext::EndTransparencyLayer() noexcept
  {
    krys_debug_assert(_transparencyLayerCount > 0);
    --_transparencyLayerCount;
  }

  SizeF GraphicsContext::DrawText(const FontCascade &font, const TextRun &run, const PointF &point,
                                  uint32 from, Maybe<uint32> to) noexcept
  {
    // Display list recording for text content is done at glyphs level. See GraphicsContext::drawGlyphs.
    return font.DrawText(*this, run, point, from, to);
  }

  void GraphicsContext::DrawBidiText(const FontCascade &font, const TextRun &run, const PointF &point,
                                     FontCascade::CustomFontNotReadyAction customFontNotReadyAction) noexcept
  {
    auto baseLevel =
      run.Direction() == text::TextDirection::RTL ? text::RightToLeftBidiLevel : text::LeftToRightBidiLevel;
    // An override draws every character in the base direction, as one run.
    auto runs = run.DirectionalOverride() ? List<text::BidiLevelRun> {{0, run.Length(), baseLevel}}
                                          : text::ResolveBidiLevelRuns(run.Text(), baseLevel);
    if (runs.empty())
    {
      return;
    }

    List<text::BidiLevel> levels;
    levels.reserve(runs.size());
    for (const auto &levelRun : runs)
    {
      levels.push_back(levelRun.Level);
    }
    List<int32> visualToLogical(runs.size());
    text::ReorderBidiRunsVisually(levels, visualToLogical);

    auto currentPoint = point;
    for (auto logicalIndex : visualToLogical)
    {
      const auto &levelRun = runs[static_cast<size_t>(logicalIndex)];
      auto subrun =
        run.SubRun(static_cast<uint32>(levelRun.Start), static_cast<uint32>(levelRun.End - levelRun.Start));
      subrun.SetDirection(levelRun.Level % 2 ? text::TextDirection::RTL : text::TextDirection::LTR);
      subrun.SetDirectionalOverride(run.DirectionalOverride());

      std::ignore = font.DrawText(*this, subrun, currentPoint, 0, null, customFontNotReadyAction);
      currentPoint.Move(font.Width(subrun), 0);
    }
  }

  void GraphicsContext::DrawEmphasisMarks(const FontCascade &font, const TextRun &run, const StringAtom &mark,
                                          const PointF &point, uint32 from, Maybe<uint32> to) noexcept
  {
    font.DrawEmphasisMarks(*this, run, mark, point, from, to);
  }

  void GraphicsContext::DrawGlyphsImmediate(const Font &font, Span<const GlyphBufferGlyph> glyphs,
                                            Span<const GlyphBufferAdvance> advances, const PointF &point,
                                            FontSmoothingMode fontSmoothingMode) noexcept
  {
    // Called by implementations that transform drawGlyphs into drawGlyphsImmediate, drawImageBuffer, etc
    // calls.
    DrawGlyphs(font, glyphs, advances, point, fontSmoothingMode);
  }

  ImageDrawResult GraphicsContext::DrawImage(Image &image, const PointF &destination,
                                             ImagePaintingOptions imagePaintingOptions) noexcept
  {
    return DrawImage(image, RectF(destination, image.Size()), RectF(PointF(), image.Size()),
                     imagePaintingOptions);
  }

  ImageDrawResult GraphicsContext::DrawImage(Image &image, const RectF &destination,
                                             ImagePaintingOptions imagePaintingOptions) noexcept
  {
    RectF srcRect(PointF(), image.Size(imagePaintingOptions.Orientation()));
    return DrawImage(image, destination, srcRect, imagePaintingOptions);
  }

  ImageDrawResult GraphicsContext::DrawImage(Image &image, const RectF &destination, const RectF &source,
                                             ImagePaintingOptions options) noexcept
  {
    InterpolationQualityMaintainer interpolationQualityForThisScope(*this, options.GetInterpolationQuality());
    return image.Draw(*this, destination, source, options);
  }

  ImageDrawResult GraphicsContext::DrawTiledImage(Image &image, const RectF &destination,
                                                  const PointF &source, const SizeF &tileSize,
                                                  const SizeF &spacing, ImagePaintingOptions options) noexcept
  {
    InterpolationQualityMaintainer interpolationQualityForThisScope(*this, options.GetInterpolationQuality());
    return image.DrawTiled(*this, destination, source, tileSize, spacing, options);
  }

  namespace
  {
    KRYS_NODISCARD SizeI ScaledImageBufferSize(const SizeF &size, const SizeF &scale) noexcept
    {
      // Enlarge the buffer size if the context's transform is scaling it so we need a higher
      // resolution than one pixel per unit.
      return ExpandedSizeI(size * scale);
    }

    KRYS_NODISCARD RectI ScaledImageBufferRect(const RectF &rect, const SizeF &scale) noexcept
    {
      auto scaledRect = rect;
      scaledRect.Scale(scale);
      return EnclosingRectI(scaledRect);
    }

    KRYS_NODISCARD SizeF ClampingScaleForImageBufferSize(const SizeF &size) noexcept
    {
      SizeF clampingScale(1, 1);
      (void)ImageBuffer::SizeNeedsClamping(size, clampingScale);
      return clampingScale;
    }
  }

  RefPtr<ImageBuffer> GraphicsContext::CreateImageBuffer(const SizeF &size,
                                                         float resolutionScale) const noexcept
  {
    return ImageBuffer::Create(size, resolutionScale);
  }

  RefPtr<ImageBuffer> GraphicsContext::CreateScaledImageBuffer(const SizeF &size,
                                                               const SizeF &scale) const noexcept
  {
    auto expandedScaledSize = ScaledImageBufferSize(size, scale);
    if (expandedScaledSize.IsEmpty())
    {
      return nullptr;
    }

    auto clampingScale = ClampingScaleForImageBufferSize(SizeF(expandedScaledSize));

    auto imageBuffer = CreateImageBuffer(SizeF(expandedScaledSize) * clampingScale, 1);
    if (!imageBuffer)
    {
      return nullptr;
    }

    imageBuffer->Context().Scale(clampingScale);

    // 'expandedScaledSize' is mapped to 'size'. So use 'expandedScaledSize / size'
    // not 'scale' because they are not necessarily equal.
    imageBuffer->Context().Scale(SizeF(expandedScaledSize) / size);
    return imageBuffer;
  }

  RefPtr<ImageBuffer> GraphicsContext::CreateScaledImageBuffer(const RectF &rect,
                                                               const SizeF &scale) const noexcept
  {
    auto expandedScaledRect = ScaledImageBufferRect(rect, scale);
    if (expandedScaledRect.IsEmpty())
    {
      return nullptr;
    }

    auto clampingScale = ClampingScaleForImageBufferSize(SizeF(expandedScaledRect.Size()));

    auto imageBuffer = CreateImageBuffer(SizeF(expandedScaledRect.Size()) * clampingScale, 1);
    if (!imageBuffer)
    {
      return nullptr;
    }

    imageBuffer->Context().Scale(clampingScale);

    // 'rect' is mapped to a rectangle inside expandedScaledRect.
    imageBuffer->Context().Translate(static_cast<float>(-expandedScaledRect.X()),
                                     static_cast<float>(-expandedScaledRect.Y()));

    // The size of this rectangle is not necessarily equal to expandedScaledRect.size().
    // So use 'scale' not 'expandedScaledRect.size() / rect.size()'.
    imageBuffer->Context().Scale(scale);
    return imageBuffer;
  }

  RefPtr<ImageBuffer> GraphicsContext::CreateAlignedImageBuffer(const SizeF &size) const noexcept
  {
    return CreateScaledImageBuffer(size, ScaleFactor());
  }

  void GraphicsContext::DrawImageBuffer(ImageBuffer &image, const PointF &destination,
                                        ImagePaintingOptions imagePaintingOptions) noexcept
  {
    DrawImageBuffer(image, RectF(destination, image.LogicalSize()), RectF({}, image.LogicalSize()),
                    imagePaintingOptions);
  }

  void GraphicsContext::DrawImageBuffer(ImageBuffer &image, const RectF &destination,
                                        ImagePaintingOptions imagePaintingOptions) noexcept
  {
    DrawImageBuffer(image, destination, RectF({}, image.LogicalSize()), imagePaintingOptions);
  }

  void GraphicsContext::DrawFilteredImageBuffer(ImageBuffer *sourceImage, const RectF &sourceImageRect,
                                                Filter &filter, FilterResults &results) noexcept
  {
    auto result = filter.Apply(sourceImage, sourceImageRect, results);
    if (!result)
    {
      return;
    }

    RefPtr imageBuffer = ShareRefPtr(filter.FilterResultBuffer(*result));
    if (!imageBuffer)
    {
      return;
    }

    Scale({1 / filter.FilterScale().Width(), 1 / filter.FilterScale().Height()});
    DrawImageBuffer(*imageBuffer, RectF(result->AbsoluteImageRect()));
    Scale(filter.FilterScale());
  }

  void GraphicsContext::DrawImageBuffer(ImageBuffer &, const RectF &, const RectF &,
                                        ImagePaintingOptions) noexcept
  {
    // FIXME: Draw the buffer's native image (nativeImageForDrawing).
  }

  void GraphicsContext::DrawPattern(ImageBuffer &, const RectF &, const RectF &, const AffineTransform &,
                                    const PointF &, const SizeF &, ImagePaintingOptions) noexcept
  {
    // FIXME: Draw the buffer's native image as a pattern (nativeImageForDrawing).
  }

  ImageDrawResult GraphicsContext::DrawTiledImage(Image &image, const RectF &destination, const RectF &source,
                                                  const SizeF &tileScaleFactor, Image::TileRule hRule,
                                                  Image::TileRule vRule,
                                                  ImagePaintingOptions options) noexcept
  {
    if (hRule == Image::StretchTile && vRule == Image::StretchTile)
    {
      // Just do a scale.
      return DrawImage(image, destination, source, options);
    }

    InterpolationQualityMaintainer interpolationQualityForThisScope(*this, options.GetInterpolationQuality());
    return image.DrawTiled(*this, destination, source, tileScaleFactor, hRule, vRule,
                           ImagePaintingOptions {options.GetCompositeOperator()});
  }

  void GraphicsContext::ClipRoundedRect(const FloatRoundedRect &rect) noexcept
  {
    Path path;
    path.AddRoundedRect(rect);
    ClipPath(path);
  }

  void GraphicsContext::ClipOutRoundedRect(const FloatRoundedRect &rect) noexcept
  {
    if (!rect.IsRounded())
    {
      ClipOut(rect.Rect());
      return;
    }

    Path path;
    path.AddRoundedRect(rect);
    ClipOut(path);
  }

  RectI GraphicsContext::ClipBounds() const noexcept
  {
    krys_debug_assert(false);
    return RectI();
  }

  void GraphicsContext::FillRect(const RectF &rect, Gradient &gradient) noexcept
  {
    // The platform fills a rect with a gradient in the fill's gradient space.
    FillRect(rect, gradient, FillGradientSpaceTransform());
  }

  void GraphicsContext::FillRect(const RectF &rect, const Color &color, CompositeOperator op,
                                 web::BlendMode blendMode) noexcept
  {
    CompositeOperator previousOperator = CompositeOperation();
    SetCompositeOperation(op, blendMode);
    FillRect(rect, color);
    SetCompositeOperation(previousOperator);
  }

  void GraphicsContext::FillRoundedRect(const FloatRoundedRect &rect, const Color &color,
                                        web::BlendMode blendMode) noexcept
  {
    if (rect.IsRounded())
    {
      SetCompositeOperation(CompositeOperation(), blendMode);
      FillRoundedRectImpl(rect, color);
      SetCompositeOperation(CompositeOperation());
    }
    else
    {
      FillRect(rect.Rect(), color, CompositeOperation(), blendMode);
    }
  }

  void GraphicsContext::FillRectWithRoundedHole(const RectF &rect, const FloatRoundedRect &roundedHoleRect,
                                                const Color &color) noexcept
  {
    Path path;
    path.AddRect(rect);

    if (!roundedHoleRect.GetRadii().IsZero())
    {
      path.AddRoundedRect(roundedHoleRect);
    }
    else
    {
      path.AddRect(roundedHoleRect.Rect());
    }

    WindRule oldFillRule = FillRule();
    Color oldFillColor = FillColor();

    SetFillRule(WindRule::EvenOdd);
    SetFillColor(color);

    FillPath(path);

    SetFillRule(oldFillRule);
    SetFillColor(oldFillColor);
  }

  SizeF GraphicsContext::ScaleFactor() const noexcept
  {
    AffineTransform transform = GetCTM(GraphicsContext::DefinitelyIncludeDeviceScale);
    return SizeF(static_cast<float>(transform.XScale()), static_cast<float>(transform.YScale()));
  }

  SizeF GraphicsContext::ScaleFactorForDrawing(const RectF &destRect, const RectF &srcRect) const noexcept
  {
    AffineTransform transform = GetCTM(GraphicsContext::DefinitelyIncludeDeviceScale);
    auto transformedDestRect = transform.MapRect(destRect);
    return transformedDestRect.Size() / srcRect.Size();
  }

  void GraphicsContext::DrawPath(const Path &path) noexcept
  {
    FillPath(path);
    StrokePath(path);
  }

  void GraphicsContext::FillEllipseAsPath(const RectF &ellipse) noexcept
  {
    Path path;
    path.AddEllipseInRect(ellipse);
    FillPath(path);
  }

  void GraphicsContext::StrokeEllipseAsPath(const RectF &ellipse) noexcept
  {
    Path path;
    path.AddEllipseInRect(ellipse);
    StrokePath(path);
  }

  void GraphicsContext::DrawLineForText(const RectF &rect, bool isPrinting, bool doubleUnderlines,
                                        web::StrokeStyle style) noexcept
  {
    SegmentF line[1] {{0, rect.Width()}};
    DrawLinesForText(rect.Location(), rect.Height(), line, isPrinting, doubleUnderlines, style);
  }

  void GraphicsContext::DrawControlPart(ControlPart &part, const FloatRoundedRect &borderRect,
                                        float deviceScaleFactor, const ControlStyle &style) noexcept
  {
    part.Draw(*this, borderRect, deviceScaleFactor, style);
  }

  void GraphicsContext::DrawDisplayList(const display_list::DisplayList &displayList) noexcept
  {
    for (auto &item : displayList.Items())
    {
      display_list::ApplyItem(*this, item);
    }
  }

  RectF GraphicsContext::ComputeUnderlineBoundsForText(const RectF &rect, bool printing) noexcept
  {
    Color dummyColor;
    return ComputeLineBoundsAndAntialiasingModeForText(rect, printing, dummyColor);
  }

  RectF GraphicsContext::ComputeLineBoundsAndAntialiasingModeForText(const RectF &rect, bool printing,
                                                                     Color &color) const noexcept
  {
    PointF origin = rect.Location();
    float thickness = std::max(rect.Height(), 0.5f);
    if (printing)
    {
      return RectF(origin, SizeF(rect.Width(), thickness));
    }

    AffineTransform transform = GetCTM(GraphicsContext::DefinitelyIncludeDeviceScale);
    // Just compute scale in x dimension, assuming x and y scales are equal.
    float scale =
      static_cast<float>(transform.B() ? std::hypot(transform.A(), transform.B()) : transform.A());
    if (scale < 1.0)
    {
      // This code always draws a line that is at least one-pixel line high,
      // which tends to visually overwhelm text at small scales. To counter this
      // effect, an alpha is applied to the underline color when text is at small scales.
      static const float minimumUnderlineAlpha = 0.4f;
      float shade = scale > minimumUnderlineAlpha ? scale : minimumUnderlineAlpha;
      color = color.ColorWithAlphaMultipliedBy(shade);
    }

    PointF devicePoint = transform.MapPoint(rect.Location());
    // Visual overflow might occur here due to integral roundf/ceilf. visualOverflowForDecorations adjusts the
    // overflow value for underline decoration.
    PointF deviceOrigin = PointF(std::roundf(devicePoint.X()), std::ceilf(devicePoint.Y()));
    if (auto inverse = transform.Inverse())
    {
      origin = inverse.value().MapPoint(deviceOrigin);
    }
    return RectF(origin, SizeF(rect.Width(), thickness));
  }

  auto GraphicsContext::DashPatternForStrokeLength(float length, float thickness, bool closedPath) noexcept
    -> Maybe<DashPattern>
  {
    const float dash = thickness * (thickness >= 3 ? 2.0f : 3.0f);
    const float gap = thickness * (thickness >= 3 ? 1.0f : 2.0f);
    if (length <= dash * 2)
    {
      return null;
    }

    // Exactly two dashes, sized in proportion.
    const float twoDashes = 2 * dash + gap + (closedPath ? gap : 0);
    if (length <= twoDashes)
    {
      const float scale = length / twoDashes;
      return DashPattern {dash * scale, gap * scale};
    }

    // Whichever number of dashes needs the gap changed the least.
    const float available = closedPath ? length : length + gap;
    const float fewerDashes = std::floor(available / (dash + gap));
    const float moreDashes = fewerDashes + 1;
    const float gapWithFewer = (length - fewerDashes * dash) / (closedPath ? fewerDashes : fewerDashes - 1);
    const float gapWithMore = (length - moreDashes * dash) / (closedPath ? moreDashes : moreDashes - 1);
    const bool useFewer = gapWithMore <= 0 || std::abs(gapWithFewer - gap) < std::abs(gapWithMore - gap);
    return DashPattern {dash, useFewer ? gapWithFewer : gapWithMore};
  }

  float GraphicsContext::DashedLineCornerWidthForStrokeWidth(float strokeWidth) const noexcept
  {
    float thickness = StrokeThickness();
    return GetStrokeStyle() == web::StrokeStyle::DottedStroke
             ? thickness
             : std::min(2.0f * thickness, std::max(thickness, strokeWidth / 3.0f));
  }

  float GraphicsContext::DashedLinePatternWidthForStrokeWidth(float strokeWidth) const noexcept
  {
    float thickness = StrokeThickness();
    return GetStrokeStyle() == web::StrokeStyle::DottedStroke
             ? thickness
             : std::min(3.0f * thickness, std::max(thickness, strokeWidth / 3.0f));
  }

  float GraphicsContext::DashedLinePatternOffsetForPatternAndStrokeWidth(float patternWidth,
                                                                         float strokeWidth) noexcept
  {
    // Pattern starts with full fill and ends with the empty fill.
    // 1. Let's start with the empty phase after the corner.
    // 2. Check if we've got odd or even number of patterns and whether they fully cover the line.
    // 3. In case of even number of patterns and/or remainder, move the pattern start position
    // so that the pattern is balanced between the corners.
    float patternOffset = patternWidth;
    int32 numberOfSegments = static_cast<int32>(std::floor(strokeWidth / patternWidth));
    bool oddNumberOfSegments = numberOfSegments % 2;
    float remainder = strokeWidth - (static_cast<float>(numberOfSegments) * patternWidth);
    if (oddNumberOfSegments && remainder)
    {
      patternOffset -= remainder / 2.0f;
    }
    else if (!oddNumberOfSegments)
    {
      if (remainder)
      {
        patternOffset += patternOffset - (patternWidth + remainder) / 2.0f;
      }
      else
      {
        patternOffset += patternWidth / 2.0f;
      }
    }

    return patternOffset;
  }

  List<PointF> GraphicsContext::CenterLineAndCutOffCorners(bool isVerticalLine, float cornerWidth,
                                                           PointF point1, PointF point2) noexcept
  {
    // Center line and cut off corners for pattern painting.
    if (isVerticalLine)
    {
      float centerOffset = (point2.X() - point1.X()) / 2.0f;
      point1.Move(centerOffset, cornerWidth);
      point2.Move(-centerOffset, -cornerWidth);
    }
    else
    {
      float centerOffset = (point2.Y() - point1.Y()) / 2.0f;
      point1.Move(cornerWidth, centerOffset);
      point2.Move(-cornerWidth, -centerOffset);
    }

    return {point1, point2};
  }

  auto GraphicsContext::ComputeRectsAndStrokeColorForLinesForText(const PointF &point, float thickness,
                                                                  Span<const SegmentF> lineSegments,
                                                                  bool isPrinting, bool doubleLines,
                                                                  web::StrokeStyle strokeStyle) noexcept
    -> RectsAndStrokeColor
  {
    RectsAndStrokeColor result;
    auto &rects = result.Rects;
    auto &strokeColor = result.StrokeColor;
    if (lineSegments.empty())
    {
      return result;
    }
    strokeColor = StrokeColor();
    RectF bounds = ComputeLineBoundsAndAntialiasingModeForText(
      RectF {point, SizeF {lineSegments.back().End, thickness}}, isPrinting, strokeColor);
    if (bounds.IsEmpty())
    {
      return result;
    }

    rects.reserve((doubleLines ? 2 : 1) * lineSegments.size());

    float dashWidth = 0;
    switch (strokeStyle)
    {
      case web::StrokeStyle::DottedStroke: dashWidth = bounds.Height(); break;
      case web::StrokeStyle::DashedStroke: dashWidth = 2 * bounds.Height(); break;
      case web::StrokeStyle::SolidStroke:
      default:                             break;
    }

    if (dashWidth)
    {
      for (const auto &lineSegment : lineSegments)
      {
        auto left = lineSegment.Begin;
        auto width = lineSegment.Length();
        auto doubleWidth = 2 * dashWidth;
        auto quotient = static_cast<int32>(left / doubleWidth);
        auto startOffset = left - static_cast<float>(quotient) * doubleWidth;
        auto effectiveLeft = left + startOffset;
        auto startParticle = static_cast<int32>(std::floor(effectiveLeft / doubleWidth));
        auto endParticle = static_cast<int32>(std::ceil((left + width) / doubleWidth));

        for (auto j = startParticle; j < endParticle; ++j)
        {
          auto actualDashWidth = dashWidth;
          auto dashStart = bounds.X() + static_cast<float>(j) * doubleWidth;

          if (j == startParticle && startOffset > 0 && startOffset < dashWidth)
          {
            actualDashWidth -= startOffset;
            dashStart += startOffset;
          }

          if (j == endParticle - 1)
          {
            auto remainingWidth = left + width - (static_cast<float>(j) * doubleWidth);
            if (remainingWidth < dashWidth)
            {
              actualDashWidth = remainingWidth;
            }
          }

          rects.push_back(RectF(dashStart, bounds.Y(), actualDashWidth, bounds.Height()));
        }
      }
    }
    else
    {
      for (const auto &lineSegment : lineSegments)
      {
        rects.push_back(
          RectF(bounds.X() + lineSegment.Begin, bounds.Y(), lineSegment.Length(), bounds.Height()));
      }
    }
    if (doubleLines)
    {
      // The space between double underlines is equal to the height of the underline.
      float y = bounds.Y() + 2 * bounds.Height();
      for (const auto &lineSegment : lineSegments)
      {
        rects.push_back(RectF(bounds.X() + lineSegment.Begin, y, lineSegment.Length(), bounds.Height()));
      }
    }
    return result;
  }
}
