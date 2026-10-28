#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Core/BidiResolver.hpp"
#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/TextRunIterator.hpp"
#include <cmath>

namespace krys::boo
{
  GraphicsContext::GraphicsContext(IsDeferred isDeferred,
                                   const GraphicsContextState::ChangeFlags &changeFlags,
                                   InterpolationQuality imageInterpolationQuality) noexcept
      : _state(changeFlags, imageInterpolationQuality), _isDeferred(isDeferred)
  {
  }

  GraphicsContext::GraphicsContext(IsDeferred isDeferred, const GraphicsContextState &state) noexcept
      : _state(state), _isDeferred(isDeferred)
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

  FloatSize GraphicsContext::PlatformShadowOffset(const FloatSize &shadowOffset) const noexcept
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

  void GraphicsContext::DrawRaisedEllipse(const FloatRect &rect, const Color &ellipseColor,
                                          const Color &shadowColor) noexcept
  {
    Save();

    SetStrokeColor(shadowColor);
    SetFillColor(shadowColor);

    DrawEllipse(FloatRect(rect.X(), rect.Y() + 1, rect.Width(), rect.Height()));

    SetStrokeColor(ellipseColor);
    SetFillColor(ellipseColor);

    DrawEllipse(rect);

    Restore();
  }

  void GraphicsContext::BeginTransparencyLayer(float) noexcept
  {
    ++_transparencyLayerCount;
  }

  void GraphicsContext::BeginTransparencyLayer(CompositeOperator, boo::BlendMode) noexcept
  {
    ++_transparencyLayerCount;
  }

  void GraphicsContext::EndTransparencyLayer() noexcept
  {
    krys_debug_assert(_transparencyLayerCount > 0);
    --_transparencyLayerCount;
  }

  FloatSize GraphicsContext::DrawText(const FontCascade &font, const TextRun &run, const FloatPoint &point,
                                      uint32 from, Maybe<uint32> to) noexcept
  {
    // Display list recording for text content is done at glyphs level. See GraphicsContext::drawGlyphs.
    return font.DrawText(*this, run, point, from, to);
  }

  void GraphicsContext::DrawBidiText(const FontCascade &font, const TextRun &run, const FloatPoint &point,
                                     FontCascade::CustomFontNotReadyAction customFontNotReadyAction) noexcept
  {
    BidiResolver<TextRunIterator, BidiCharacterRun> bidiResolver;
    bidiResolver.SetStatus(BidiStatus(run.Direction(), run.DirectionalOverride()));
    bidiResolver.SetPositionIgnoringNestedIsolates(TextRunIterator(&run, 0));

    // FIXME: This ownership should be reversed. We should pass BidiRunList
    // to BidiResolver in createBidiRunsForLine.
    auto &bidiRuns = bidiResolver.Runs();
    bidiResolver.CreateBidiRunsForLine(TextRunIterator(&run, run.Length()));

    if (!bidiRuns.RunCount())
    {
      return;
    }

    auto currentPoint = point;
    auto *bidiRun = bidiRuns.FirstRun();
    while (bidiRun)
    {
      auto subrun = run.SubRun(bidiRun->Start(), bidiRun->Stop() - bidiRun->Start());
      bool isRTL = bidiRun->Level() % 2;
      subrun.SetDirection(isRTL ? TextDirection::RTL : TextDirection::LTR);
      subrun.SetDirectionalOverride(bidiRun->DirOverride(false));

      std::ignore = font.DrawText(*this, subrun, currentPoint, 0, null, customFontNotReadyAction);
      currentPoint.Move(font.Width(subrun), 0);

      bidiRun = bidiRun->Next();
    }

    bidiRuns.Clear();
  }

  void GraphicsContext::DrawEmphasisMarks(const FontCascade &font, const TextRun &run, const StringAtom &mark,
                                          const FloatPoint &point, uint32 from, Maybe<uint32> to) noexcept
  {
    font.DrawEmphasisMarks(*this, run, mark, point, from, to);
  }

  void GraphicsContext::DrawGlyphsImmediate(const Font &font, Span<const GlyphBufferGlyph> glyphs,
                                            Span<const GlyphBufferAdvance> advances, const FloatPoint &point,
                                            FontSmoothingMode fontSmoothingMode) noexcept
  {
    // Called by implementations that transform drawGlyphs into drawGlyphsImmediate, drawImageBuffer, etc
    // calls.
    DrawGlyphs(font, glyphs, advances, point, fontSmoothingMode);
  }

  ImageDrawResult GraphicsContext::DrawImage(Image &image, const FloatPoint &destination,
                                             ImagePaintingOptions imagePaintingOptions) noexcept
  {
    return DrawImage(image, FloatRect(destination, image.Size()), FloatRect(FloatPoint(), image.Size()),
                     imagePaintingOptions);
  }

  ImageDrawResult GraphicsContext::DrawImage(Image &image, const FloatRect &destination,
                                             ImagePaintingOptions imagePaintingOptions) noexcept
  {
    FloatRect srcRect(FloatPoint(), image.Size(imagePaintingOptions.Orientation()));
    return DrawImage(image, destination, srcRect, imagePaintingOptions);
  }

  ImageDrawResult GraphicsContext::DrawImage(Image &image, const FloatRect &destination,
                                             const FloatRect &source, ImagePaintingOptions options) noexcept
  {
    InterpolationQualityMaintainer interpolationQualityForThisScope(*this, options.GetInterpolationQuality());
    return image.Draw(*this, destination, source, options);
  }

  ImageDrawResult GraphicsContext::DrawTiledImage(Image &image, const FloatRect &destination,
                                                  const FloatPoint &source, const FloatSize &tileSize,
                                                  const FloatSize &spacing,
                                                  ImagePaintingOptions options) noexcept
  {
    InterpolationQualityMaintainer interpolationQualityForThisScope(*this, options.GetInterpolationQuality());
    return image.DrawTiled(*this, destination, source, tileSize, spacing, options);
  }

  namespace
  {
    KRYS_NODISCARD IntSize ScaledImageBufferSize(const FloatSize &size, const FloatSize &scale) noexcept
    {
      // Enlarge the buffer size if the context's transform is scaling it so we need a higher
      // resolution than one pixel per unit.
      return ExpandedIntSize(size * scale);
    }

    KRYS_NODISCARD IntRect ScaledImageBufferRect(const FloatRect &rect, const FloatSize &scale) noexcept
    {
      auto scaledRect = rect;
      scaledRect.Scale(scale);
      return EnclosingIntRect(scaledRect);
    }

    KRYS_NODISCARD FloatSize ClampingScaleForImageBufferSize(const FloatSize &size) noexcept
    {
      FloatSize clampingScale(1, 1);
      (void)ImageBuffer::SizeNeedsClamping(size, clampingScale);
      return clampingScale;
    }
  }

  RefPtr<ImageBuffer> GraphicsContext::CreateImageBuffer(const FloatSize &size,
                                                         float resolutionScale) const noexcept
  {
    return ImageBuffer::Create(size, resolutionScale);
  }

  RefPtr<ImageBuffer> GraphicsContext::CreateScaledImageBuffer(const FloatSize &size,
                                                               const FloatSize &scale) const noexcept
  {
    auto expandedScaledSize = ScaledImageBufferSize(size, scale);
    if (expandedScaledSize.IsEmpty())
    {
      return nullptr;
    }

    auto clampingScale = ClampingScaleForImageBufferSize(FloatSize(expandedScaledSize));

    auto imageBuffer = CreateImageBuffer(FloatSize(expandedScaledSize) * clampingScale, 1);
    if (!imageBuffer)
    {
      return nullptr;
    }

    imageBuffer->Context().Scale(clampingScale);

    // 'expandedScaledSize' is mapped to 'size'. So use 'expandedScaledSize / size'
    // not 'scale' because they are not necessarily equal.
    imageBuffer->Context().Scale(FloatSize(expandedScaledSize) / size);
    return imageBuffer;
  }

  RefPtr<ImageBuffer> GraphicsContext::CreateScaledImageBuffer(const FloatRect &rect,
                                                               const FloatSize &scale) const noexcept
  {
    auto expandedScaledRect = ScaledImageBufferRect(rect, scale);
    if (expandedScaledRect.IsEmpty())
    {
      return nullptr;
    }

    auto clampingScale = ClampingScaleForImageBufferSize(FloatSize(expandedScaledRect.Size()));

    auto imageBuffer = CreateImageBuffer(FloatSize(expandedScaledRect.Size()) * clampingScale, 1);
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

  RefPtr<ImageBuffer> GraphicsContext::CreateAlignedImageBuffer(const FloatSize &size) const noexcept
  {
    return CreateScaledImageBuffer(size, ScaleFactor());
  }

  void GraphicsContext::DrawImageBuffer(ImageBuffer &image, const FloatPoint &destination,
                                        ImagePaintingOptions imagePaintingOptions) noexcept
  {
    DrawImageBuffer(image, FloatRect(destination, image.LogicalSize()), FloatRect({}, image.LogicalSize()),
                    imagePaintingOptions);
  }

  void GraphicsContext::DrawImageBuffer(ImageBuffer &image, const FloatRect &destination,
                                        ImagePaintingOptions imagePaintingOptions) noexcept
  {
    DrawImageBuffer(image, destination, FloatRect({}, image.LogicalSize()), imagePaintingOptions);
  }

  void GraphicsContext::DrawFilteredImageBuffer(ImageBuffer *sourceImage, const FloatRect &sourceImageRect,
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
    DrawImageBuffer(*imageBuffer, FloatRect(result->AbsoluteImageRect()));
    Scale(filter.FilterScale());
  }

  void GraphicsContext::DrawImageBuffer(ImageBuffer &, const FloatRect &, const FloatRect &,
                                        ImagePaintingOptions) noexcept
  {
    // FIXME: Draw the buffer's native image (nativeImageForDrawing), once buffers have pixels.
  }

  void GraphicsContext::DrawPattern(ImageBuffer &, const FloatRect &, const FloatRect &,
                                    const AffineTransform &, const FloatPoint &, const FloatSize &,
                                    ImagePaintingOptions) noexcept
  {
    // FIXME: Draw the buffer's native image as a pattern (nativeImageForDrawing), once buffers have pixels.
  }

  ImageDrawResult GraphicsContext::DrawTiledImage(Image &image, const FloatRect &destination,
                                                  const FloatRect &source, const FloatSize &tileScaleFactor,
                                                  Image::TileRule hRule, Image::TileRule vRule,
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

  IntRect GraphicsContext::ClipBounds() const noexcept
  {
    krys_debug_assert(false);
    return IntRect();
  }

  void GraphicsContext::FillRect(const FloatRect &rect, Gradient &gradient) noexcept
  {
    // The platform fills a rect with a gradient in the fill's gradient space.
    FillRect(rect, gradient, FillGradientSpaceTransform());
  }

  void GraphicsContext::FillRect(const FloatRect &rect, const Color &color, CompositeOperator op,
                                 boo::BlendMode blendMode) noexcept
  {
    CompositeOperator previousOperator = CompositeOperation();
    SetCompositeOperation(op, blendMode);
    FillRect(rect, color);
    SetCompositeOperation(previousOperator);
  }

  void GraphicsContext::FillRoundedRect(const FloatRoundedRect &rect, const Color &color,
                                        boo::BlendMode blendMode) noexcept
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

  void GraphicsContext::FillRectWithRoundedHole(const FloatRect &rect,
                                                const FloatRoundedRect &roundedHoleRect,
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

  FloatSize GraphicsContext::ScaleFactor() const noexcept
  {
    AffineTransform transform = GetCTM(GraphicsContext::DefinitelyIncludeDeviceScale);
    return FloatSize(static_cast<float>(transform.XScale()), static_cast<float>(transform.YScale()));
  }

  FloatSize GraphicsContext::ScaleFactorForDrawing(const FloatRect &destRect,
                                                   const FloatRect &srcRect) const noexcept
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

  void GraphicsContext::FillEllipseAsPath(const FloatRect &ellipse) noexcept
  {
    Path path;
    path.AddEllipseInRect(ellipse);
    FillPath(path);
  }

  void GraphicsContext::StrokeEllipseAsPath(const FloatRect &ellipse) noexcept
  {
    Path path;
    path.AddEllipseInRect(ellipse);
    StrokePath(path);
  }

  void GraphicsContext::DrawLineForText(const FloatRect &rect, bool isPrinting, bool doubleUnderlines,
                                        boo::StrokeStyle style) noexcept
  {
    FloatSegment line[1] {{0, rect.Width()}};
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

  FloatRect GraphicsContext::ComputeUnderlineBoundsForText(const FloatRect &rect, bool printing) noexcept
  {
    Color dummyColor;
    return ComputeLineBoundsAndAntialiasingModeForText(rect, printing, dummyColor);
  }

  FloatRect GraphicsContext::ComputeLineBoundsAndAntialiasingModeForText(const FloatRect &rect, bool printing,
                                                                         Color &color) noexcept
  {
    FloatPoint origin = rect.Location();
    float thickness = std::max(rect.Height(), 0.5f);
    if (printing)
    {
      return FloatRect(origin, FloatSize(rect.Width(), thickness));
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

    FloatPoint devicePoint = transform.MapPoint(rect.Location());
    // Visual overflow might occur here due to integral roundf/ceilf. visualOverflowForDecorations adjusts the
    // overflow value for underline decoration.
    FloatPoint deviceOrigin = FloatPoint(std::roundf(devicePoint.X()), std::ceilf(devicePoint.Y()));
    if (auto inverse = transform.Inverse())
    {
      origin = inverse.value().MapPoint(deviceOrigin);
    }
    return FloatRect(origin, FloatSize(rect.Width(), thickness));
  }

  float GraphicsContext::DashedLineCornerWidthForStrokeWidth(float strokeWidth) const noexcept
  {
    float thickness = StrokeThickness();
    return GetStrokeStyle() == boo::StrokeStyle::DottedStroke
             ? thickness
             : std::min(2.0f * thickness, std::max(thickness, strokeWidth / 3.0f));
  }

  float GraphicsContext::DashedLinePatternWidthForStrokeWidth(float strokeWidth) const noexcept
  {
    float thickness = StrokeThickness();
    return GetStrokeStyle() == boo::StrokeStyle::DottedStroke
             ? thickness
             : std::min(3.0f * thickness, std::max(thickness, strokeWidth / 3.0f));
  }

  float GraphicsContext::DashedLinePatternOffsetForPatternAndStrokeWidth(float patternWidth,
                                                                         float strokeWidth) const noexcept
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

  List<FloatPoint> GraphicsContext::CenterLineAndCutOffCorners(bool isVerticalLine, float cornerWidth,
                                                               FloatPoint point1,
                                                               FloatPoint point2) const noexcept
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

  auto GraphicsContext::ComputeRectsAndStrokeColorForLinesForText(const FloatPoint &point, float thickness,
                                                                  Span<const FloatSegment> lineSegments,
                                                                  bool isPrinting, bool doubleLines,
                                                                  boo::StrokeStyle strokeStyle) noexcept
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
    FloatRect bounds = ComputeLineBoundsAndAntialiasingModeForText(
      FloatRect {point, FloatSize {lineSegments.back().End, thickness}}, isPrinting, strokeColor);
    if (bounds.IsEmpty())
    {
      return result;
    }

    rects.reserve((doubleLines ? 2 : 1) * lineSegments.size());

    float dashWidth = 0;
    switch (strokeStyle)
    {
      case boo::StrokeStyle::DottedStroke: dashWidth = bounds.Height(); break;
      case boo::StrokeStyle::DashedStroke: dashWidth = 2 * bounds.Height(); break;
      case boo::StrokeStyle::SolidStroke:
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

          rects.push_back(FloatRect(dashStart, bounds.Y(), actualDashWidth, bounds.Height()));
        }
      }
    }
    else
    {
      for (const auto &lineSegment : lineSegments)
      {
        rects.push_back(
          FloatRect(bounds.X() + lineSegment.Begin, bounds.Y(), lineSegment.Length(), bounds.Height()));
      }
    }
    if (doubleLines)
    {
      // The space between double underlines is equal to the height of the underline.
      float y = bounds.Y() + 2 * bounds.Height();
      for (const auto &lineSegment : lineSegments)
      {
        rects.push_back(FloatRect(bounds.X() + lineSegment.Begin, y, lineSegment.Length(), bounds.Height()));
      }
    }
    return result;
  }
}
