#include "Krystal.Booey/Rendering/TextDecorationPainter.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/TextBoxPainter.hpp"

namespace krys::boo
{
  KRYS_NODISCARD static StrokeStyle
    TextDecorationStyleToStrokeStyle(TextDecorationStyle decorationStyle) noexcept
  {
    StrokeStyle strokeStyle = StrokeStyle::SolidStroke;
    switch (decorationStyle)
    {
      case TextDecorationStyle::Solid:  strokeStyle = StrokeStyle::SolidStroke; break;
      case TextDecorationStyle::Double: strokeStyle = StrokeStyle::DoubleStroke; break;
      case TextDecorationStyle::Dotted: strokeStyle = StrokeStyle::DottedStroke; break;
      case TextDecorationStyle::Dashed: strokeStyle = StrokeStyle::DashedStroke; break;
      case TextDecorationStyle::Wavy:   strokeStyle = StrokeStyle::WavyStroke; break;
    }

    return strokeStyle;
  }

  static void AdjustLineToPixelBoundaries(FloatPoint &p1, FloatPoint &p2, float strokeWidth,
                                          StrokeStyle penStyle) noexcept
  {
    // For odd widths, we add in 0.5 to the appropriate x/y so that the float arithmetic
    // works out. For example, with a border width of 3, we are passed (y1+y2)/2, e.g.,
    // (50+53)/2 = 103/2 = 51 when we want 51.5. It is always true that an even width gave
    // us a perfect position, but an odd width gave us a position that is off by exactly 0.5.
    if (penStyle == StrokeStyle::DottedStroke || penStyle == StrokeStyle::DashedStroke)
    {
      if (p1.X() == p2.X())
      {
        p1.SetY(p1.Y() + strokeWidth);
        p2.SetY(p2.Y() - strokeWidth);
      }
      else
      {
        p1.SetX(p1.X() + strokeWidth);
        p2.SetX(p2.X() - strokeWidth);
      }
    }

    if (static_cast<int32>(strokeWidth) % 2)
    {
      if (p1.X() == p2.X())
      {
        // We're a vertical line. Adjust our x.
        p1.SetX(p1.X() + 0.5f);
        p2.SetX(p2.X() + 0.5f);
      }
      else
      {
        // We're a horizontal line. Adjust our y.
        p1.SetY(p1.Y() + 0.5f);
        p2.SetY(p2.Y() + 0.5f);
      }
    }
  }

  /*
   * Draw one cubic Bezier curve and repeat the same pattern along the the decoration's axis.
   * The start point (p1), controlPoint1, controlPoint2 and end point (p2) of the Bezier curve
   * form a diamond shape:
   *
   *                              step
   *                         |-----------|
   *
   *                   controlPoint1
   *                         +
   *
   *
   *                  . .
   *                .     .
   *              .         .
   * (x1, y1) p1 +           .            + p2 (x2, y2) - <--- Decoration's axis
   *                          .         .               |
   *                            .     .                 |
   *                              . .                   | controlPointDistance
   *                                                    |
   *                                                    |
   *                         +                          -
   *                   controlPoint2
   *
   *             |-----------|
   *                 step
   */
  static void StrokeWavyTextDecoration(GraphicsContext &context, const FloatRect &rect, bool isPrinting,
                                       WavyStrokeParameters wavyStrokeParameters,
                                       StrokeStyle strokeStyle) noexcept
  {
    if (rect.IsEmpty() || !wavyStrokeParameters.Step)
    {
      return;
    }

    // 1. Calculate the endpoints.
    FloatPoint p1 = rect.MinXMinYCorner();
    FloatPoint p2 = rect.MaxXMinYCorner();

    // Extent the wavy line before and after the text so it can cover the whole length.
    p1.SetX(p1.X() - 2 * wavyStrokeParameters.Step);
    p2.SetX(p2.X() + 2 * wavyStrokeParameters.Step);

    AdjustLineToPixelBoundaries(p1, p2, rect.Height(), context.GetStrokeStyle());

    krys_debug_assert(p1.Y() == p2.Y());
    float x1 = std::min(p1.X(), p2.X());
    float x2 = std::max(p1.X(), p2.X());

    // Ensure the wavy underline path will not have too many segments.
    constexpr uint32 maxTextDecorationWaves = 1'024;
    if (wavyStrokeParameters.Step < 1 || (x2 - x1) / (2 * wavyStrokeParameters.Step) > maxTextDecorationWaves)
    {
      context.DrawLineForText(rect, isPrinting, false, strokeStyle);
      return;
    }

    // 2. Contruct the wavy underline path.
    float yAxis = p1.Y();
    FloatPoint controlPoint1(0, yAxis + wavyStrokeParameters.ControlPointDistance);
    FloatPoint controlPoint2(0, yAxis - wavyStrokeParameters.ControlPointDistance);

    Path path;
    path.MoveTo(p1);

    for (double x = x1; x + 2 * wavyStrokeParameters.Step <= x2;)
    {
      controlPoint1.SetX(static_cast<float>(x + wavyStrokeParameters.Step));
      controlPoint2.SetX(static_cast<float>(x + wavyStrokeParameters.Step));
      x += 2 * wavyStrokeParameters.Step;
      path.AddBezierCurveTo(controlPoint1, controlPoint2, FloatPoint(static_cast<float>(x), yAxis));
    }

    // Offset the bounds and set extra height to ensure the whole wavy line is covered.
    auto bounds = rect;
    bounds.InflateY(wavyStrokeParameters.ControlPointDistance);

    // 3. Draw the wavy underline path and clip the extra wavy line added before.
    GraphicsContextStateSaver stateSaver(context);
    context.Clip(bounds);

    context.SetShouldAntialias(true);
    context.SetStrokeThickness(rect.Height());
    context.StrokePath(path);
  }

  bool TextDecorationPainter::Styles::operator==(const Styles &other) const noexcept
  {
    return Underline.DecorationColor == other.Underline.DecorationColor
           && Overline.DecorationColor == other.Overline.DecorationColor
           && Linethrough.DecorationColor == other.Linethrough.DecorationColor
           && Underline.DecorationStyle == other.Underline.DecorationStyle
           && Overline.DecorationStyle == other.Overline.DecorationStyle
           && Linethrough.DecorationStyle == other.Linethrough.DecorationStyle;
  }

  TextDecorationPainter::TextDecorationPainter(GraphicsContext &context, const FontCascade &font,
                                               const style::TextShadows &shadow, bool isPrinting,
                                               WritingMode writingMode) noexcept
      : _context(context), _isPrinting(isPrinting), _writingMode(writingMode), _shadow(shadow), _font(font)
  {
  }

  // Paint text-shadow, underline, overline
  void TextDecorationPainter::PaintBackgroundDecorations(
    const RenderStyle &style, const TextRun &textRun, const BackgroundDecorationGeometry &decorationGeometry,
    style::TextDecorationLine decorationType, const Styles &decorationStyle) noexcept
  {
    using Flag = style::TextDecorationLine::Flag;

    auto paintDecoration =
      [&](Flag decoration, TextDecorationStyle underlineStyle, const Color &color, const FloatRect &rect)
    {
      _context.SetStrokeColor(color);

      auto strokeStyle = TextDecorationStyleToStrokeStyle(underlineStyle);

      if (underlineStyle == TextDecorationStyle::Wavy)
      {
        StrokeWavyTextDecoration(_context, rect, _isPrinting, decorationGeometry.WavyParameters, strokeStyle);
      }
      else if (decoration == Flag::Underline || decoration == Flag::Overline)
      {
        if ((style.GetTextDecorationSkipInk() == TextDecorationSkipInk::Auto
             || style.GetTextDecorationSkipInk() == TextDecorationSkipInk::All)
            && !_writingMode.IsVerticalTypographic())
        {
          if (!_context.PaintingDisabled())
          {
            auto underlineBoundingBox = _context.ComputeUnderlineBoundsForText(rect, _isPrinting);
            auto intersections = _font.LineSegmentsForIntersectionsWithRect(
              textRun, decorationGeometry.TextOrigin, underlineBoundingBox);
            if (!intersections.empty())
            {
              auto dilationAmount =
                std::min(underlineBoundingBox.Height(), style.MetricsOfPrimaryFont().Height() / 5);
              auto boundaries =
                DifferenceWithDilation({0, rect.Width()}, krys::move(intersections), dilationAmount);
              // We don't use underlineBoundingBox here because drawLinesForText() will run
              // computeUnderlineBoundsForText() internally.
              _context.DrawLinesForText(rect.Location(), rect.Height(), boundaries, _isPrinting,
                                        underlineStyle == TextDecorationStyle::Double, strokeStyle);
            }
            else
            {
              _context.DrawLineForText(rect, _isPrinting, underlineStyle == TextDecorationStyle::Double,
                                       strokeStyle);
            }
          }
        }
        else
        {
          // FIXME: Need to support text-decoration-skip: none.
          _context.DrawLineForText(rect, _isPrinting, underlineStyle == TextDecorationStyle::Double,
                                   strokeStyle);
        }
      }
      else
      {
        krys_debug_assert(false);
      }
    };

    auto areLinesOpaque =
      !_isPrinting && (!decorationType.HasUnderline() || decorationStyle.Underline.DecorationColor.IsOpaque())
      && (!decorationType.HasOverline() || decorationStyle.Overline.DecorationColor.IsOpaque())
      && (!decorationType.HasLineThrough() || decorationStyle.Linethrough.DecorationColor.IsOpaque());

    float extraOffset = 0.f;
    auto boxOrigin = decorationGeometry.BoxOrigin;
    bool clipping = _shadow.size() > 1 && !areLinesOpaque;
    if (clipping)
    {
      auto clipRect =
        FloatRect {boxOrigin, FloatSize {decorationGeometry.TextBoxWidth, decorationGeometry.ClippingOffset}};
      const auto &zoomFactor = style.UsedZoomForLength();
      for (const auto &shadow : _shadow)
      {
        auto shadowExtent = style::PaintingExtent(shadow, zoomFactor);
        auto shadowRect = clipRect;
        shadowRect.Inflate(shadowExtent);
        auto shadowOffset = TextBoxPainter::RotateShadowOffset(shadow.Location, _writingMode, zoomFactor);
        shadowRect.Move(shadowOffset);
        clipRect.Unite(shadowRect);
        extraOffset =
          std::max(extraOffset, std::max(0.f, shadowOffset.Height()) + static_cast<float>(shadowExtent));
      }
      _context.Save();
      _context.Clip(clipRect);
      extraOffset += decorationGeometry.ClippingOffset;
      boxOrigin.Move(0.f, extraOffset);
    }

    // These decorations should match the visual overflows computed in visualOverflowForDecorations().
    auto underlineRect = FloatRect {
      boxOrigin, FloatSize {decorationGeometry.TextBoxWidth, decorationGeometry.TextDecorationThickness}};
    auto overlineRect = underlineRect;
    if (decorationType.HasUnderline())
    {
      underlineRect.Move(0.f, decorationGeometry.UnderlineOffset);
    }
    if (decorationType.HasOverline())
    {
      overlineRect.Move(0.f, decorationGeometry.OverlineOffset);
    }

    auto draw = [&](const style::TextShadow *shadow)
    {
      if (decorationType.HasUnderline() && !underlineRect.IsEmpty())
      {
        paintDecoration(Flag::Underline, decorationStyle.Underline.DecorationStyle,
                        decorationStyle.Underline.DecorationColor, underlineRect);
      }
      if (decorationType.HasOverline() && !overlineRect.IsEmpty())
      {
        paintDecoration(Flag::Overline, decorationStyle.Overline.DecorationStyle,
                        decorationStyle.Overline.DecorationColor, overlineRect);
      }
      // We only want to paint the shadow, hence the transparent color, not the actual line-through,
      // which will be painted in paintForegroundDecorations().
      if (shadow && decorationType.HasLineThrough())
      {
        PaintLineThrough({boxOrigin, decorationGeometry.TextBoxWidth,
                          decorationGeometry.TextDecorationThickness, decorationGeometry.LinethroughCenter,
                          decorationGeometry.WavyParameters},
                         Color::transparentBlack, decorationStyle);
      }
    };

    if (_shadow.IsNone())
    {
      draw(nullptr);
    }
    else
    {
      const auto &zoomFactor = style.UsedZoomForLength();
      for (const auto &shadow : _shadow)
      {
        if (&shadow == &_shadow.last())
        {
          // The last set of lines paints normally inside the clip.
          boxOrigin.Move(0, -extraOffset);
          extraOffset = 0;
        }
        auto shadowColor = style.ColorWithColorFilter(shadow.Color);

        auto shadowOffset = TextBoxPainter::RotateShadowOffset(shadow.Location, _writingMode, zoomFactor);
        shadowOffset.Expand(0, -extraOffset);
        _context.SetDropShadow(
          {shadowOffset, shadow.Blur.ResolveZoom(zoomFactor), shadowColor, ShadowRadiusMode::Default});

        draw(&shadow);
      }
    }

    if (clipping)
    {
      _context.Restore();
    }
    else if (!_shadow.IsNone())
    {
      _context.ClearDropShadow();
    }
  }

  void TextDecorationPainter::PaintForegroundDecorations(
    const ForegroundDecorationGeometry &foregroundDecorationGeometry, const Styles &decorationStyle) noexcept
  {
    PaintLineThrough(foregroundDecorationGeometry, decorationStyle.Linethrough.DecorationColor,
                     decorationStyle);
  }

  void
    TextDecorationPainter::PaintLineThrough(const ForegroundDecorationGeometry &foregroundDecorationGeometry,
                                            const Color &color, const Styles &decorationStyle) noexcept
  {
    auto rect = FloatRect {foregroundDecorationGeometry.BoxOrigin,
                           FloatSize {foregroundDecorationGeometry.TextBoxWidth,
                                      foregroundDecorationGeometry.TextDecorationThickness}};
    rect.Move(0.f, foregroundDecorationGeometry.LinethroughCenter);

    _context.SetStrokeColor(color);

    TextDecorationStyle lineStyle = decorationStyle.Linethrough.DecorationStyle;
    auto strokeStyle = TextDecorationStyleToStrokeStyle(lineStyle);

    if (lineStyle == TextDecorationStyle::Wavy)
    {
      StrokeWavyTextDecoration(_context, rect, _isPrinting, foregroundDecorationGeometry.WavyParameters,
                               strokeStyle);
    }
    else
    {
      _context.DrawLineForText(rect, _isPrinting, lineStyle == TextDecorationStyle::Double, strokeStyle);
    }
  }

  KRYS_NODISCARD static bool IsAnchor(const dom::Node *node) noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(node);
    return element && Is<html::HTMLAnchorElement>(*element);
  }

  static void CollectStylesForRenderer(TextDecorationPainter::Styles &result, const RenderObject &renderer,
                                       style::TextDecorationLine remainingDecorations, bool firstLineStyle,
                                       EnumSet<PaintBehavior> paintBehavior) noexcept
  {
    using Flag = style::TextDecorationLine::Flag;

    auto extractDecorations = [&](const RenderStyle &style, style::TextDecorationLine decorations)
    {
      if (!decorations.ContainsAny({Flag::Underline, Flag::Overline, Flag::LineThrough}))
      {
        return;
      }

      auto color = TextDecorationPainter::DecorationColor(style, paintBehavior);
      auto decorationStyle = style.GetTextDecorationStyle();

      if (decorations.HasUnderline())
      {
        remainingDecorations.Remove(Flag::Underline);
        result.Underline.DecorationColor = color;
        result.Underline.DecorationStyle = decorationStyle;
      }
      if (decorations.HasOverline())
      {
        remainingDecorations.Remove(Flag::Overline);
        result.Overline.DecorationColor = color;
        result.Overline.DecorationStyle = decorationStyle;
      }
      if (decorations.HasLineThrough())
      {
        remainingDecorations.Remove(Flag::LineThrough);
        result.Linethrough.DecorationColor = color;
        result.Linethrough.DecorationStyle = decorationStyle;
      }
    };

    auto styleForRenderer = [&](const RenderObject &renderer) -> const RenderStyle &
    {
      return firstLineStyle ? renderer.FirstLineStyle() : renderer.Style();
    };

    const RenderObject *current = &renderer;
    do
    {
      const auto &style = styleForRenderer(*current);
      extractDecorations(style, style.GetTextDecorationLine());

      if (current->Style().Display() == DisplayType::RubyAnnotation)
      {
        return;
      }

      current = current->Parent();
      // FIXME: Anonymous blocks of inline continuations carry on at the continuation, once there are
      // continuations.

      if (remainingDecorations.IsNone())
      {
        break;
      }

    } while (current && !IsAnchor(current->GetNode()));

    // If we bailed out, use the element we bailed out at (typically a <font> or <a> element).
    if (!remainingDecorations.IsNone() && current)
    {
      extractDecorations(styleForRenderer(*current), remainingDecorations);
    }
  }

  Color TextDecorationPainter::DecorationColor(const RenderStyle &style,
                                               EnumSet<PaintBehavior> paintBehavior) noexcept
  {
    if (paintBehavior.Contains(PaintBehavior::ForceBlackText))
    {
      return Color::black;
    }

    if (paintBehavior.Contains(PaintBehavior::ForceWhiteText))
    {
      return Color::white;
    }

    return style.VisitedDependentColorWithColorFilter(css::PropertyId::TextDecorationColor, paintBehavior);
  }

  TextDecorationPainter::Styles TextDecorationPainter::StylesForRenderer(
    const RenderObject &renderer, style::TextDecorationLine requestedDecorations, bool firstLineStyle,
    EnumSet<PaintBehavior> paintBehavior) noexcept
  {
    if (requestedDecorations.IsNone())
    {
      return {};
    }

    Styles result;
    CollectStylesForRenderer(result, renderer, requestedDecorations, false, paintBehavior);
    if (firstLineStyle)
    {
      CollectStylesForRenderer(result, renderer, requestedDecorations, true, paintBehavior);
    }
    return result;
  }

  style::TextDecorationLine TextDecorationPainter::TextDecorationsInEffectForStyle(
    const TextDecorationPainter::Styles &style) noexcept
  {
    using Flag = style::TextDecorationLine::Flag;

    EnumSet<Flag> decorations;
    if (style.Underline.DecorationColor.IsValid())
    {
      decorations.Add(Flag::Underline);
    }
    if (style.Overline.DecorationColor.IsValid())
    {
      decorations.Add(Flag::Overline);
    }
    if (style.Linethrough.DecorationColor.IsValid())
    {
      decorations.Add(Flag::LineThrough);
    }
    return decorations;
  }
}
