/*
 * Copyright (C) 2024 Igalia S.L.
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

#include "Krystal.Booey/GraphicsContextRender2D.hpp"
#include "Krystal.Booey/FilterGraph2DBuilder.hpp"
#include "Krystal.Booey/ImageBufferTextures.hpp"
#include "Krystal.Booey/Render2DConversions.hpp"
#include "Krystal.Web/Graphics/Filters/Filter.hpp"
#include "Krystal.Web/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Web/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Web/Graphics/Fonts/Font.hpp"
#include "Krystal.Web/Graphics/Gradient.hpp"
#include "Krystal.Web/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Web/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Web/Graphics/Images/NativeImage.hpp"
#include "Krystal.Web/Graphics/Paths/Path.hpp"
#include <cmath>
#include <limits>
#include <numbers>

namespace krys::boo
{
  namespace
  {
    /// @brief The operator Render2D draws @p op with, blending with @p blendMode as Skia does: a blend mode
    /// other than normal blends over. Plus darker and difference aren't Porter-Duff operators: they draw
    /// over.
    KRYS_NODISCARD gfx::CompositeOperator2D ToCompositeOperator2D(web::CompositeOperator op,
                                                                  web::BlendMode blendMode) noexcept
    {
      using Op = gfx::CompositeOperator2D;
      if (blendMode != web::BlendMode::Normal)
      {
        return Op::SourceOver;
      }
      switch (op)
      {
        case web::CompositeOperator::Clear:           return Op::Clear;
        case web::CompositeOperator::Copy:            return Op::Copy;
        case web::CompositeOperator::SourceIn:        return Op::SourceIn;
        case web::CompositeOperator::SourceOut:       return Op::SourceOut;
        case web::CompositeOperator::SourceAtop:      return Op::SourceAtop;
        case web::CompositeOperator::DestinationOver: return Op::DestinationOver;
        case web::CompositeOperator::DestinationIn:   return Op::DestinationIn;
        case web::CompositeOperator::DestinationOut:  return Op::DestinationOut;
        case web::CompositeOperator::DestinationAtop: return Op::DestinationAtop;
        case web::CompositeOperator::XOR:             return Op::Xor;
        case web::CompositeOperator::PlusLighter:     return Op::PlusLighter;
        // FIXME: Plus darker and difference as compositing operators, once Render2D has them.
        default:                                      return Op::SourceOver;
      }
    }

    KRYS_NODISCARD bool IsValidDashArray(const web::DashArray &dashArray) noexcept
    {
      // See 'dom-context-2d-setlinedash': if the array contains not finite or negative values, return.
      web::DashArrayElement total = 0;
      for (const auto &dash : dashArray)
      {
        if (dash < 0 || !std::isfinite(dash))
        {
          return false;
        }
        total += dash;
      }
      // Nothing to be done for all-zero or empty arrays.
      return total > 0;
    }

    // Creates a path comprising of two triangle waves separated by some empty space in Y axis.
    // The empty space can be filled, thus forming an elegant triangle wave.
    // Such triangle wave can be used e.g. as an error underline for text.
    KRYS_NODISCARD gfx::Path2D CreateErrorUnderlinePath(const RectF &boundaries) noexcept
    {
      const double y = boundaries.Y();
      double width = boundaries.Width();
      const double height = boundaries.Height();
      static const double heightSquares = 2.5;

      const double square = height / heightSquares;
      const double halfSquare = 0.5 * square;

      const double unitWidth = (heightSquares - 1.0) * square;
      const int32 widthUnits = static_cast<int32>((width + 0.5 * unitWidth) / unitWidth);

      double x = boundaries.X() + 0.5 * (width - widthUnits * unitWidth);
      width = widthUnits * unitWidth;

      const double bottom = y + height;
      const double top = y;

      auto point = [](double px, double py)
      {
        return Vec2f {static_cast<float>(px), static_cast<float>(py)};
      };
      gfx::Path2D path;

      // Bottom triangle wave, left to right.
      path.MoveTo(point(x - halfSquare, top + halfSquare));

      int32 i = 0;
      for (i = 0; i < widthUnits; i += 2)
      {
        const double middle = x + (i + 1) * unitWidth;
        const double right = x + (i + 2) * unitWidth;

        path.LineTo(point(middle, bottom));

        if (i + 2 == widthUnits)
        {
          path.LineTo(point(right + halfSquare, top + halfSquare));
        }
        else if (i + 1 != widthUnits)
        {
          path.LineTo(point(right, top + square));
        }
      }

      // Top triangle wave, right to left.
      for (i -= 2; i >= 0; i -= 2)
      {
        const double left = x + i * unitWidth;
        const double middle = x + (i + 1) * unitWidth;
        const double right = x + (i + 2) * unitWidth;

        if (i + 1 == widthUnits)
        {
          path.LineTo(point(middle + halfSquare, bottom - halfSquare));
        }
        else
        {
          if (i + 2 == widthUnits)
          {
            path.LineTo(point(right, top));
          }

          path.LineTo(point(middle, bottom - halfSquare));
        }

        path.LineTo(point(left, top));
      }

      return path;
    }
  }

  GraphicsContextRender2D::GraphicsContextRender2D(gfx::Renderer2D &renderer, FontFaceCache &fonts,
                                                   const RectF &bounds,
                                                   ImageBufferTextures *imageBuffers) noexcept
      : _renderer(renderer), _fonts(fonts), _imageBuffers(imageBuffers), _clipBounds(bounds),
        _initialClipBounds(bounds), _initialClip(renderer.GetClip())
  {
    _renderer.Save();
  }

  GraphicsContextRender2D::~GraphicsContextRender2D() noexcept
  {
    UnwindStateStack();
    while (IsInTransparencyLayer())
    {
      EndTransparencyLayer();
    }
    EndImageClips();
    _renderer.Restore();
  }

  void GraphicsContextRender2D::Save(web::GraphicsContextState::Purpose purpose) noexcept
  {
    GraphicsContext::Save(purpose);
    _strokeStack.push_back(_stroke);
    _clipBoundsStack.push_back(_clipBounds);
    _renderer.Save();
  }

  void GraphicsContextRender2D::Restore(web::GraphicsContextState::Purpose purpose) noexcept
  {
    if (!StackSize())
    {
      return;
    }

    EndImageClips();
    GraphicsContext::Restore(purpose);

    if (!_strokeStack.empty())
    {
      _stroke = krys::move(_strokeStack.back());
      _strokeStack.pop_back();
    }
    if (!_clipBoundsStack.empty())
    {
      _clipBounds = _clipBoundsStack.back();
      _clipBoundsStack.pop_back();
    }

    _renderer.Restore();
  }

  // Draws a filled rectangle with a stroked border.
  void GraphicsContextRender2D::DrawRect(const RectF &rect, float borderThickness) noexcept
  {
    krys_debug_assert(!rect.IsEmpty());
    PrepareDraw();
    _renderer.FillRect(rect, FillBrushPaint().Paint());
    if (GetStrokeStyle() == web::StrokeStyle::NoStroke)
    {
      return;
    }

    const RectF rects[4] = {
      {rect.X(), rect.Y(), rect.Width(), borderThickness},
      {rect.X(), rect.MaxY() - borderThickness, rect.Width(), borderThickness},
      {rect.X(), rect.Y() + borderThickness, borderThickness, rect.Height() - 2 * borderThickness},
      {rect.MaxX() - borderThickness, rect.Y() + borderThickness, borderThickness,
       rect.Height() - 2 * borderThickness}};
    const auto stroke = StrokeBrushPaint();
    for (const auto &side : rects)
    {
      _renderer.FillRect(side, stroke.Paint());
    }
  }

  // This is only used to draw borders, so we should not draw shadows.
  void GraphicsContextRender2D::DrawLine(const PointF &point1, const PointF &point2) noexcept
  {
    if (GetStrokeStyle() == web::StrokeStyle::NoStroke)
    {
      return;
    }

    const bool isVertical = (point1.X() + StrokeThickness() == point2.X());
    float strokeWidth = isVertical ? point2.Y() - point1.Y() : point2.X() - point1.X();
    if (!StrokeThickness() || !strokeWidth)
    {
      return;
    }

    auto style = CurrentStrokeStyle(StrokeThickness());
    style.Dashes.clear();
    float cornerWidth = 0;

    if (GetStrokeStyle() == web::StrokeStyle::DashedStroke)
    {
      // From end to end, with no room for dashes leaving it solid.
      if (auto pattern = DashPatternForStrokeLength(strokeWidth, StrokeThickness(), false))
      {
        style.Dashes = {pattern->Dash, pattern->Gap};
        style.DashOffset = 0;
      }
    }
    else if (GetStrokeStyle() == web::StrokeStyle::DottedStroke)
    {
      // Figure out end points to ensure we always paint corners.
      cornerWidth = DashedLineCornerWidthForStrokeWidth(strokeWidth);
      if (isVertical)
      {
        FillRect(RectF(point1.X(), point1.Y(), StrokeThickness(), cornerWidth), StrokeColor());
        FillRect(RectF(point1.X(), point2.Y() - cornerWidth, StrokeThickness(), cornerWidth), StrokeColor());
      }
      else
      {
        FillRect(RectF(point1.X(), point1.Y(), cornerWidth, StrokeThickness()), StrokeColor());
        FillRect(RectF(point2.X() - cornerWidth, point1.Y(), cornerWidth, StrokeThickness()), StrokeColor());
      }
      strokeWidth -= 2 * cornerWidth;
      const float patternWidth = DashedLinePatternWidthForStrokeWidth(strokeWidth);
      // Check if corner drawing sufficiently covers the line.
      if (strokeWidth <= patternWidth + 1)
      {
        return;
      }

      style.Dashes = {patternWidth, patternWidth};
      style.DashOffset = DashedLinePatternOffsetForPatternAndStrokeWidth(patternWidth, strokeWidth);
    }

    const auto centeredPoints = CenterLineAndCutOffCorners(isVertical, cornerWidth, point1, point2);
    PrepareDraw(false);
    _renderer.StrokeLine(ToVec2f(centeredPoints[0]), ToVec2f(centeredPoints[1]), style,
                         ToColour(StrokeColor(), Alpha()));
  }

  // This method is only used to draw the little circles used in lists.
  void GraphicsContextRender2D::DrawEllipse(const RectF &boundaries) noexcept
  {
    gfx::Path2D path;
    path.AddEllipse(ToVec2f(boundaries.Center()), {boundaries.Width() / 2.f, boundaries.Height() / 2.f});
    PrepareDraw();
    FillShape(path, gfx::FillRule::NonZero, FillBrushPaint().Paint());
  }

  void GraphicsContextRender2D::FillPath(const web::Path &path) noexcept
  {
    if (path.IsEmpty())
    {
      return;
    }

    PrepareDraw();
    FillShape(ToPath2D(path), ToFillRule(FillRule()), FillBrushPaint().Paint());
  }

  void GraphicsContextRender2D::StrokePath(const web::Path &path) noexcept
  {
    if (path.IsEmpty())
    {
      return;
    }

    PrepareDraw();
    _renderer.StrokePath(ToPath2D(path), CurrentStrokeStyle(StrokeThickness()), StrokeBrushPaint().Paint());
  }

  void GraphicsContextRender2D::FillRect(const RectF &boundaries, RequiresClipToRect) noexcept
  {
    PrepareDraw();
    _renderer.FillRect(boundaries, FillBrushPaint().Paint());
  }

  void GraphicsContextRender2D::FillRect(const RectF &boundaries, const web::Color &fillColor) noexcept
  {
    PrepareDraw();
    _renderer.FillRect(boundaries, ToColour(fillColor));
  }

  void GraphicsContextRender2D::FillRect(const RectF &boundaries, web::Gradient &gradient,
                                         const AffineTransform &gradientSpaceTransform,
                                         RequiresClipToRect) noexcept
  {
    PrepareDraw();
    const auto gradient2D = ToGradient2D(gradient, Alpha());
    _renderer.FillRect(boundaries, gfx::Paint2D(gradient2D, gradientSpaceTransform));
  }

  void GraphicsContextRender2D::FillRoundedRectImpl(const web::FloatRoundedRect &rect,
                                                    const web::Color &color) noexcept
  {
    PrepareDraw();
    if (auto circular = ToCircularRoundedRect(rect))
    {
      _renderer.FillRoundedRect(*circular, ToColour(color));
      return;
    }

    gfx::Path2D path;
    AddRoundedRect(path, rect);
    _renderer.FillPath(path, gfx::FillRule::NonZero, ToColour(color));
  }

  void GraphicsContextRender2D::FillRectWithRoundedHole(const RectF &outerRect,
                                                        const web::FloatRoundedRect &roundedHoleRect,
                                                        const web::Color &color) noexcept
  {
    if (!color.IsValid())
    {
      return;
    }

    auto fill = [&]
    {
      if (auto circular = ToCircularRoundedRect(roundedHoleRect))
      {
        _renderer.FillRectWithRoundedHole(outerRect, *circular, ToColour(color));
        return;
      }

      gfx::Path2D path;
      path.AddRect(outerRect);
      AddRoundedRect(path, roundedHoleRect);
      _renderer.FillPath(path, gfx::FillRule::EvenOdd, ToColour(color));
    };

    // With a shadow (an inset one), only the shadow is drawn: the shape is moved out of view and its shadow
    // back into place.
    PrepareDraw();
    if (!HasDropShadow())
    {
      fill();
      return;
    }

    constexpr float away = 100'000.f;
    const auto transform = _renderer.GetTransform();
    auto shadow = *DropShadow();
    if (ShadowsIgnoreTransforms())
    {
      shadow.Offset.SetWidth(shadow.Offset.Width() - away);
    }
    else if (auto inverse = transform.Inverse())
    {
      shadow.Offset = shadow.Offset - inverse->MapSize(SizeF {away, 0});
    }
    _renderer.Save();
    _renderer.SetTransform(AffineTransform::CreateTranslation(SizeF(away, 0.f)) * transform);
    _renderer.SetDropShadow({.Offset = {shadow.Offset.Width(), shadow.Offset.Height()},
                             .BlurRadius = shadow.Radius,
                             .Colour = ToColour(shadow.Color)});
    fill();
    _renderer.Restore();
  }

  void GraphicsContextRender2D::ClearRect(const RectF &rect) noexcept
  {
    // As Skia clears: the rect, transformed and clipped, becomes transparent black, whatever the alpha,
    // shadow and compositing.
    _renderer.Save();
    _renderer.ClearDropShadow();
    _renderer.SetBlendMode(BlendMode::Normal);
    _renderer.SetCompositeOperator(gfx::CompositeOperator2D::Clear);
    _renderer.FillRect(rect, SRGBA<uint8> {0, 0, 0, 255});
    _renderer.Restore();
  }

  void GraphicsContextRender2D::StrokeRect(const RectF &boundaries, float lineWidth) noexcept
  {
    gfx::Path2D path;
    path.AddRect(boundaries);
    PrepareDraw();
    _renderer.StrokePath(path, CurrentStrokeStyle(lineWidth), StrokeBrushPaint().Paint());
  }

  void GraphicsContextRender2D::SetLineCap(web::LineCap lineCap) noexcept
  {
    _stroke.Cap = ToLineCap(lineCap);
  }

  void GraphicsContextRender2D::SetLineDash(const web::DashArray &dashArray, float dashOffset) noexcept
  {
    _stroke.Dashes.clear();
    _stroke.DashOffset = 0.f;
    if (!IsValidDashArray(dashArray))
    {
      return;
    }

    // Repeat the array to ensure even number of dash array elements, see e.g. 'stroke-dasharray' spec.
    const size_t repeats = dashArray.size() % 2 == 1 ? 2 : 1;
    for (size_t i = 0; i < repeats; ++i)
    {
      _stroke.Dashes.append_range(dashArray);
    }
    _stroke.DashOffset = dashOffset;
  }

  void GraphicsContextRender2D::SetLineJoin(web::LineJoin lineJoin) noexcept
  {
    _stroke.Join = ToLineJoin(lineJoin);
  }

  void GraphicsContextRender2D::SetMiterLimit(float miter) noexcept
  {
    _stroke.MiterLimit = miter;
  }

  void GraphicsContextRender2D::ResetClip() noexcept
  {
    _clipBounds = _initialClipBounds;
    _renderer.SetClip(_initialClip);
  }

  void GraphicsContextRender2D::Clip(const RectF &rect) noexcept
  {
    IntersectClipBounds(rect);
    _renderer.ClipRect(rect);
  }

  void GraphicsContextRender2D::ClipRoundedRect(const web::FloatRoundedRect &rect) noexcept
  {
    IntersectClipBounds(rect.Rect());
    if (auto circular = ToCircularRoundedRect(rect))
    {
      _renderer.ClipRoundedRect(*circular);
      return;
    }

    gfx::Path2D path;
    AddRoundedRect(path, rect);
    _renderer.ClipPath(path, gfx::FillRule::NonZero);
  }

  void GraphicsContextRender2D::ClipOut(const RectF &rect) noexcept
  {
    _renderer.ClipOutRect(rect);
  }

  void GraphicsContextRender2D::ClipOut(const web::Path &path) noexcept
  {
    _renderer.ClipOutPath(ToPath2D(path), gfx::FillRule::NonZero);
  }

  void GraphicsContextRender2D::ClipOutRoundedRect(const web::FloatRoundedRect &rect) noexcept
  {
    if (auto circular = ToCircularRoundedRect(rect))
    {
      _renderer.ClipOutRoundedRect(*circular);
      return;
    }

    gfx::Path2D path;
    AddRoundedRect(path, rect);
    _renderer.ClipOutPath(path, gfx::FillRule::NonZero);
  }

  void GraphicsContextRender2D::ClipPath(const web::Path &path, web::WindRule clipRule) noexcept
  {
    IntersectClipBounds(path.FastBoundingRect());
    _renderer.ClipPath(ToPath2D(path), ToFillRule(clipRule));
  }

  void GraphicsContextRender2D::ClipToImageBuffer(web::ImageBuffer &image, const RectF &destination) noexcept
  {
    // As Skia's clip shader, which leaves nothing outside the image. Render2D can't clip to a texture, so
    // what's drawn until this state ends goes into a layer, which the image's alpha masks as it ends.
    IntersectClipBounds(destination);
    _renderer.ClipRect(destination);

    auto *commands = _renderer.GetCommandBuffer();
    const auto texture =
      _imageBuffers && commands ? _imageBuffers->Render(image, *commands) : gfx::TextureHandle {};
    if (!texture.IsValid())
    {
      IntersectClipBounds({});
      _renderer.ClipRect({});
      return;
    }

    PrepareDraw(false);
    _imageClips.push_back({.Texture = texture,
                           .Destination = RectF {destination.Location(), image.LogicalSize()},
                           .Transform = _renderer.GetTransform(),
                           .StackSize = StackSize()});
    _renderer.BeginLayer(1.f);
  }

  RectI GraphicsContextRender2D::ClipBounds() const noexcept
  {
    if (auto inverse = GetCTM().Inverse())
    {
      return EnclosingRectI(inverse->MapRect(_clipBounds));
    }
    return EnclosingRectI(_clipBounds);
  }

  /// @brief Appends @p path to @p into, moved by @p offset.
  static void AppendTranslated(gfx::Path2D &into, const gfx::Path2D &path, Vec2f offset) noexcept
  {
    auto points = path.Points();
    size_t next = 0;
    for (auto verb : path.Verbs())
    {
      switch (verb)
      {
        case gfx::PathVerb::MoveTo:
          into.MoveTo(points[next] + offset);
          next += 1;
          break;
        case gfx::PathVerb::LineTo:
          into.LineTo(points[next] + offset);
          next += 1;
          break;
        case gfx::PathVerb::QuadTo:
          into.QuadTo(points[next] + offset, points[next + 1] + offset);
          next += 2;
          break;
        case gfx::PathVerb::CubicTo:
          into.CubicTo(points[next] + offset, points[next + 1] + offset, points[next + 2] + offset);
          next += 3;
          break;
        case gfx::PathVerb::Close: into.Close(); break;
      }
    }
  }

  void GraphicsContextRender2D::DrawGlyphs(const web::Font &font, Span<const web::GlyphBufferGlyph> glyphs,
                                           Span<const web::GlyphBufferAdvance> advances,
                                           const PointF &position, web::FontSmoothingMode) noexcept
  {
    const auto mode = GetTextDrawingMode();
    const bool fill = mode.contains(web::TextDrawingMode::Fill);
    const bool stroke = mode.contains(web::TextDrawingMode::Stroke);
    if ((!fill && !stroke) || glyphs.empty())
    {
      return;
    }
    const auto *face = _fonts.FaceFor(font);
    if (!face)
    {
      return;
    }

    // Each glyph's pen is the sum of the advances before it.
    List<uint32> glyphIndices;
    List<Vec2f> positions;
    glyphIndices.reserve(glyphs.size());
    positions.reserve(glyphs.size());
    Vec2f pen {};
    for (size_t i = 0; i < glyphs.size(); ++i)
    {
      glyphIndices.push_back(glyphs[i]);
      positions.push_back(pen);
      if (i < advances.size())
      {
        pen += Vec2f {advances[i].Width(), advances[i].Height()};
      }
    }

    const bool isVertical = font.PlatformData().Orientation() == web::FontOrientation::Vertical;
    if (isVertical)
    {
      _renderer.Save();
      _renderer.Translate(position.X(), position.Y());
      _renderer.Rotate(-std::numbers::pi_v<float> / 2.f);
      _renderer.Translate(-position.X(), -position.Y());
    }

    PrepareDraw();
    const float size = font.PlatformData().Size();
    if (fill)
    {
      _renderer.DrawGlyphs(*face, size, glyphIndices, positions, ToVec2f(position), FillBrushPaint().Paint());
    }
    if (stroke)
    {
      // Stroked glyphs are their outlines, stroked as one path.
      gfx::Path2D outlines;
      for (size_t i = 0; i < glyphIndices.size(); ++i)
      {
        AppendTranslated(outlines, face->GlyphPath(glyphIndices[i], size), ToVec2f(position) + positions[i]);
      }
      if (!outlines.IsEmpty())
      {
        _renderer.StrokePath(outlines, CurrentStrokeStyle(StrokeThickness()), StrokeBrushPaint().Paint());
      }
    }

    if (isVertical)
    {
      _renderer.Restore();
    }
  }

  void GraphicsContextRender2D::DrawLinesForText(const PointF &origin, float thickness,
                                                 Span<const SegmentF> lineSegments, bool printing,
                                                 bool doubleUnderlines, web::StrokeStyle strokeStyle) noexcept
  {
    auto [rects, strokeColor] = ComputeRectsAndStrokeColorForLinesForText(
      origin, thickness, lineSegments, printing, doubleUnderlines, strokeStyle);
    for (auto &rect : rects)
    {
      FillRect(rect, strokeColor);
    }
  }

  void GraphicsContextRender2D::DrawDotsForDocumentMarker(const RectF &boundaries,
                                                          web::DocumentMarkerLineStyle style) noexcept
  {
    if (style.Mode != web::DocumentMarkerLineStyleMode::Spelling
        && style.Mode != web::DocumentMarkerLineStyleMode::Grammar)
    {
      return;
    }

    PrepareDraw(false);
    _renderer.FillPath(CreateErrorUnderlinePath(boundaries), gfx::FillRule::NonZero, ToColour(style.Color));
  }

  void GraphicsContextRender2D::BeginTransparencyLayer(float opacity) noexcept
  {
    GraphicsContext::BeginTransparencyLayer(opacity);
    _layerStack.emplace_back();
    _clipBoundsStack.push_back(_clipBounds);
    PrepareDraw(false);

    const auto &style = Style();
    if (!style)
    {
      _renderer.BeginLayer(opacity);
      return;
    }

    // The style filters the layer as it's composited, in target pixels; what's drawn inside isn't styled.
    const auto ctm = GetCTM();
    const float scale = static_cast<float>((ctm.XScale() + ctm.YScale()) / 2.);
    const auto filter = krys::SwitchOn(
      *style,
      [](const web::GraphicsColorMatrix &matrix) { return gfx::LayerFilter2D::ColourMatrix(matrix.Values); },
      [&](const web::GraphicsGaussianBlur &blur)
      {
        // A layer blurs evenly: an uneven blur uses the larger radius.
        return gfx::LayerFilter2D::Blur(std::max(blur.Radius.Width(), blur.Radius.Height()) * scale);
      },
      [&](const web::GraphicsDropShadow &shadow)
      {
        const auto offset = ctm.MapSize(shadow.Offset);
        return gfx::LayerFilter2D::DropShadow(
          {.Offset = {offset.Width(), offset.Height()},
           .BlurRadius = shadow.Radius * scale,
           .Colour = ToColour(shadow.Color.ColorWithAlphaMultipliedBy(shadow.Opacity))});
      });
    _renderer.BeginLayer(opacity, _renderer.GetBlendMode(), filter);
    SetStyle(null);
  }

  void GraphicsContextRender2D::BeginTransparencyLayer(web::CompositeOperator operation,
                                                       web::BlendMode blendMode) noexcept
  {
    GraphicsContext::BeginTransparencyLayer(operation, blendMode);
    _layerStack.push_back({web::CompositeMode {operation, blendMode}});
    _clipBoundsStack.push_back(_clipBounds);
    _renderer.SetCompositeOperator(ToCompositeOperator2D(operation, blendMode));
    _renderer.BeginLayer(1.f, blendMode);
    // When on transparency layer, we don't want to blend operations as when layer ends, we blend it as a
    // whole.
    SetCompositeMode({web::CompositeOperator::SourceOver, BlendMode::Normal});
  }

  void GraphicsContextRender2D::EndTransparencyLayer() noexcept
  {
    EndImageClips();
    GraphicsContext::EndTransparencyLayer();
    _renderer.EndLayer();
    if (!_clipBoundsStack.empty())
    {
      _clipBounds = _clipBoundsStack.back();
      _clipBoundsStack.pop_back();
    }
    if (!_layerStack.empty())
    {
      auto layerState = _layerStack.back();
      _layerStack.pop_back();
      if (layerState.CompositeMode)
      {
        SetCompositeMode(*layerState.CompositeMode);
      }
    }
  }

  void GraphicsContextRender2D::DrawFocusRing(const web::Path &path, float, const web::Color &color) noexcept
  {
    // As the Adwaita theme paints focus (Skia's backend draws nothing without it): a ring of a fixed width
    // outside the path, with round joins, at the colour's alpha. Adwaita strokes the path twice as wide and
    // clears its inside; Render2D has no clear operator, so the inside is clipped out instead.
    constexpr float focusLineWidth = 2.f;

    PrepareDraw(false);
    const auto path2D = ToPath2D(path);
    _renderer.BeginLayer(color.AlphaAsFloat());
    _renderer.ClipOutPath(path2D, gfx::FillRule::NonZero);
    _renderer.StrokePath(
      path2D, {.Width = focusLineWidth * 2, .Cap = gfx::LineCap::Round, .Join = gfx::LineJoin::Round},
      ToColour(color.OpaqueColor()));
    _renderer.EndLayer();
  }

  void GraphicsContextRender2D::DrawFocusRing(const List<RectF> &rects, float, float outlineWidth,
                                              const web::Color &color) noexcept
  {
    // Adwaita rounds each rect's corners by 2.
    web::Path path;
    for (const auto &rect : rects)
    {
      path.AddRoundedRect(rect, SizeF(2, 2));
    }
    DrawFocusRing(path, outlineWidth, color);
  }

  void GraphicsContextRender2D::Scale(const SizeF &scale) noexcept
  {
    _renderer.Scale(scale.Width(), scale.Height());
  }

  void GraphicsContextRender2D::Rotate(float radians) noexcept
  {
    _renderer.Rotate(radians);
  }

  void GraphicsContextRender2D::Translate(float x, float y) noexcept
  {
    _renderer.Translate(x, y);
  }

  void GraphicsContextRender2D::ConcatCTM(const AffineTransform &transform) noexcept
  {
    _renderer.ConcatTransform(transform);
  }

  void GraphicsContextRender2D::SetCTM(const AffineTransform &transform) noexcept
  {
    _renderer.SetTransform(transform);
  }

  AffineTransform GraphicsContextRender2D::GetCTM(IncludeDeviceScale) const noexcept
  {
    return _renderer.GetTransform();
  }

  GraphicsContextRender2D::Brush GraphicsContextRender2D::FillBrushPaint() const noexcept
  {
    const auto &brush = FillBrush();
    if (auto *pattern = brush.GetPattern())
    {
      return PatternBrushPaint(*pattern);
    }
    if (auto *gradient = brush.GetGradient())
    {
      return {.Gradient = ToGradient2D(*gradient, Alpha()), .GradientSpace = brush.GradientSpaceTransform()};
    }
    return {.Colour = ToColour(brush.GetColor(), Alpha())};
  }

  GraphicsContextRender2D::Brush GraphicsContextRender2D::StrokeBrushPaint() const noexcept
  {
    const auto &brush = StrokeBrush();
    if (auto *pattern = brush.GetPattern())
    {
      return PatternBrushPaint(*pattern);
    }
    if (auto *gradient = brush.GetGradient())
    {
      return {.Gradient = ToGradient2D(*gradient, Alpha()), .GradientSpace = brush.GradientSpaceTransform()};
    }
    return {.Colour = ToColour(brush.GetColor(), Alpha())};
  }

  GraphicsContextRender2D::Brush
    GraphicsContextRender2D::PatternBrushPaint(const web::Pattern &pattern) const noexcept
  {
    // As Skia's: the whole tile image is the tile, placed by the pattern space transform.
    auto *commands = _renderer.GetCommandBuffer();
    if (!_imageBuffers || !commands)
    {
      return {};
    }
    gfx::TextureHandle texture;
    if (auto *imageBuffer = pattern.TileImage().ImageBufferIfExists())
    {
      texture = _imageBuffers->Render(*imageBuffer, *commands);
    }
    else if (auto *nativeImage = pattern.TileImage().NativeImageIfExists())
    {
      texture = _imageBuffers->Upload(*nativeImage);
    }
    if (!texture.IsValid())
    {
      return {};
    }
    const auto size = pattern.TileImage().Size();
    return {.Pattern = gfx::ImagePattern2D {
              .Texture = texture,
              .PatternSpace = pattern.PatternSpaceTransform(),
              .TileSize = {static_cast<float>(size.Width()), static_cast<float>(size.Height())},
              .Filter = ImageFilter(),
              .Opacity = Alpha(),
              .RepeatX = pattern.RepeatX(),
              .RepeatY = pattern.RepeatY()}};
  }

  gfx::StrokeStyle GraphicsContextRender2D::CurrentStrokeStyle(float width) const noexcept
  {
    return {.Width = width,
            .Cap = _stroke.Cap,
            .Join = _stroke.Join,
            .MiterLimit = _stroke.MiterLimit,
            .Dashes = _stroke.Dashes,
            .DashOffset = _stroke.DashOffset};
  }

  gfx::SamplerFilter GraphicsContextRender2D::ImageFilter() const noexcept
  {
    // Skia samples without interpolating only when asked not to.
    return ImageInterpolationQuality() == web::InterpolationQuality::DoNotInterpolate
             ? gfx::SamplerFilter::Nearest
             : gfx::SamplerFilter::Linear;
  }

  void GraphicsContextRender2D::DrawNativeImage(web::NativeImage &image, const RectF &destRect,
                                                const RectF &srcRect,
                                                web::ImagePaintingOptions options) noexcept
  {
    auto *commands = _renderer.GetCommandBuffer();
    if (!_imageBuffers || !commands)
    {
      return;
    }

    auto orientation = options.Orientation();
    auto imageSize = SizeF(image.Size());
    if (orientation.UsesWidthAsHeight())
    {
      imageSize = imageSize.TransposedSize();
    }
    auto normalizedSrcRect = NormaliseRect(srcRect);
    if (!RectF({}, imageSize).Intersects(normalizedSrcRect))
    {
      return;
    }
    if (orientation.UsesWidthAsHeight())
    {
      normalizedSrcRect = normalizedSrcRect.TransposedRect();
    }

    const auto texture = _imageBuffers->Upload(image);
    if (!texture.IsValid())
    {
      return;
    }

    auto normalizedDestRect = NormaliseRect(destRect);
    web::GraphicsContextStateSaver stateSaver(*this, orientation != img::ImageOrientation::Orientation::None);
    if (orientation != img::ImageOrientation::Orientation::None)
    {
      // ImageOrientation expects the origin to be at (0, 0).
      Translate(normalizedDestRect.X(), normalizedDestRect.Y());
      normalizedDestRect.SetLocation(PointF());
      ConcatCTM(orientation.TransformFromDefault(normalizedDestRect.Size()));

      if (orientation.UsesWidthAsHeight())
      {
        // The destination rectangle will have its width and height already reversed for the orientation of
        // the image, as it was needed for page layout, so we need to reverse it back here.
        normalizedDestRect.SetSize(normalizedDestRect.Size().TransposedSize());
      }
    }

    const auto &platformImage = image.GetPlatformImage();
    const auto alpha = platformImage->External() && !platformImage->PremultipliedAlpha()
                         ? AlphaPremultiplication::Unpremultiplied
                         : AlphaPremultiplication::Premultiplied;
    DrawTexture(texture, normalizedSrcRect, normalizedDestRect, options, alpha);
  }

  void GraphicsContextRender2D::DrawPattern(web::NativeImage &image, const RectF &destRect,
                                            const RectF &tileRect, const AffineTransform &patternTransform,
                                            const PointF &phase, const SizeF &spacing,
                                            web::ImagePaintingOptions options) noexcept
  {
    auto *commands = _renderer.GetCommandBuffer();
    if (!_imageBuffers || !commands || !patternTransform.IsInvertible() || tileRect.IsEmpty())
    {
      return;
    }
    const auto texture = _imageBuffers->Upload(image);
    if (!texture.IsValid())
    {
      return;
    }
    DrawTexturePattern(texture, SizeF(image.Size()), destRect, tileRect, patternTransform, phase, spacing,
                       options, *commands);
  }

  void GraphicsContextRender2D::DrawImageBuffer(web::ImageBuffer &image, const RectF &destination,
                                                const RectF &source,
                                                web::ImagePaintingOptions options) noexcept
  {
    auto *commands = _renderer.GetCommandBuffer();
    if (!_imageBuffers || !commands)
    {
      return;
    }
    const auto texture = _imageBuffers->Render(image, *commands);
    if (!texture.IsValid())
    {
      return;
    }

    RectF sourceScaled = source;
    sourceScaled.Scale(image.ResolutionScale());
    DrawTexture(texture, sourceScaled, destination, options);
  }

  void GraphicsContextRender2D::DrawFilteredImageBuffer(web::ImageBuffer *sourceImage,
                                                        const RectF &sourceImageRect, web::Filter &filter,
                                                        web::FilterResults &) noexcept
  {
    auto *commands = _renderer.GetCommandBuffer();
    if (!_imageBuffers || !commands)
    {
      return;
    }
    auto *filters = _imageBuffers->Filters();
    if (!filters)
    {
      return;
    }

    // The effects' results are nodes of the graph rather than pixels, so the results are this draw's own.
    FilterGraph2DBuilder builder(*_imageBuffers, *commands);
    web::FilterResults results([&](const web::FilterEffect &effect)
                               { return builder.CreateApplier(effect); });
    auto result = filter.Apply(sourceImage, sourceImageRect, results);
    if (!result || !builder.Finish(*result))
    {
      return;
    }

    auto rendered = filters->Render(*commands, builder.Graph());
    if (!rendered)
    {
      return;
    }

    const auto &region = rendered->Region;
    const RectF source(0.f, 0.f, static_cast<float>(region.Width()), static_cast<float>(region.Height()));
    Scale({1 / filter.FilterScale().Width(), 1 / filter.FilterScale().Height()});
    DrawTexture(rendered->Texture, source, RectF(result->AbsoluteImageRect()), {});
    Scale(filter.FilterScale());
  }

  void GraphicsContextRender2D::DrawPattern(web::ImageBuffer &image, const RectF &destRect,
                                            const RectF &tileRect, const AffineTransform &patternTransform,
                                            const PointF &phase, const SizeF &spacing,
                                            web::ImagePaintingOptions options) noexcept
  {
    auto *commands = _renderer.GetCommandBuffer();
    if (!_imageBuffers || !commands || !patternTransform.IsInvertible() || tileRect.IsEmpty())
    {
      return;
    }
    const auto texture = _imageBuffers->Render(image, *commands);
    if (!texture.IsValid())
    {
      return;
    }
    DrawTexturePattern(texture, SizeF(image.BackendSize()), destRect, tileRect, patternTransform, phase,
                       spacing, options, *commands);
  }

  void GraphicsContextRender2D::DrawTexture(gfx::TextureHandle texture, const RectF &source,
                                            const RectF &destination, web::ImagePaintingOptions options,
                                            AlphaPremultiplication alpha) noexcept
  {
    web::InterpolationQualityMaintainer interpolationQualityForThisScope(*this,
                                                                         options.GetInterpolationQuality());
    PrepareDraw();
    _renderer.Save();
    PrepareImageDraw(options);
    _renderer.DrawImage(texture, source, destination,
                        {.Filter = ImageFilter(), .Alpha = alpha, .Opacity = Alpha()});
    _renderer.Restore();
  }

  void GraphicsContextRender2D::DrawTexturePattern(gfx::TextureHandle texture, const SizeF &size,
                                                   const RectF &destRect, const RectF &tileRect,
                                                   const AffineTransform &patternTransform,
                                                   const PointF &phase, const SizeF &spacing,
                                                   web::ImagePaintingOptions options,
                                                   gfx::CommandBuffer &commands) noexcept
  {
    // As Skia's: the shader places the tile at the phase, moved by the tile's place in the image under the
    // pattern transform's scale. The tile is the whole texture, so a tile that's part of the image (or has
    // spacing after it) is copied into a texture of its own, its size rounded out to whole texels.
    PointF phaseOffset(tileRect.Location());
    phaseOffset.Scale(static_cast<float>(patternTransform.A()), static_cast<float>(patternTransform.D()));
    phaseOffset.MoveBy(phase);
    const auto shaderMatrix =
      AffineTransform::CreateTranslation(SizeF(phaseOffset.X(), phaseOffset.Y())) * patternTransform;

    auto tileTexture = texture;
    bool repeatX = true;
    bool repeatY = true;
    if (spacing.IsZero() && tileRect.Size() == size)
    {
      // Repeat only along an axis the destination reaches past the image on; otherwise clamp, so a tile
      // scaled to fit the destination exactly doesn't filter in the texels at its other end.
      if (const auto inverse = shaderMatrix.Inverse())
      {
        const auto corners = {gfx::MapPoint(*inverse, {destRect.X(), destRect.Y()}),
                              gfx::MapPoint(*inverse, {destRect.MaxX(), destRect.Y()}),
                              gfx::MapPoint(*inverse, {destRect.X(), destRect.MaxY()}),
                              gfx::MapPoint(*inverse, {destRect.MaxX(), destRect.MaxY()})};
        float left = std::numeric_limits<float>::max();
        float top = left;
        float right = std::numeric_limits<float>::lowest();
        float bottom = right;
        for (const auto &corner : corners)
        {
          left = std::min(left, corner.x);
          top = std::min(top, corner.y);
          right = std::max(right, corner.x);
          bottom = std::max(bottom, corner.y);
        }
        repeatX = left < 0.f || std::trunc(right) > size.Width();
        repeatY = top < 0.f || std::trunc(bottom) > size.Height();
      }
    }
    else
    {
      const auto enclosing = EnclosingRectI(tileRect);
      tileTexture = _imageBuffers->Copy(texture, tileRect, static_cast<uint32>(enclosing.Width()),
                                        static_cast<uint32>(enclosing.Height()), commands);
      if (!tileTexture.IsValid())
      {
        return;
      }
    }

    web::InterpolationQualityMaintainer interpolationQualityForThisScope(*this,
                                                                         options.GetInterpolationQuality());
    // Spacing is in the destination's units; the pattern's is in the tile's.
    const gfx::ImagePattern2D pattern {
      .Texture = tileTexture,
      .PatternSpace = shaderMatrix,
      .TileSize = {tileRect.Width(), tileRect.Height()},
      .Spacing = {spacing.Width() / static_cast<float>(patternTransform.A()),
                  spacing.Height() / static_cast<float>(patternTransform.D())},
      .Filter = ImageFilter(),
      .Opacity = Alpha(),
      .RepeatX = repeatX,
      .RepeatY = repeatY};

    PrepareDraw();
    _renderer.Save();
    PrepareImageDraw(options);
    _renderer.FillRect(destRect, pattern);
    _renderer.Restore();
  }

  void GraphicsContextRender2D::PrepareImageDraw(const web::ImagePaintingOptions &options) noexcept
  {
    _renderer.SetBlendMode(options.GetBlendMode());
    _renderer.SetCompositeOperator(
      ToCompositeOperator2D(options.GetCompositeOperator(), options.GetBlendMode()));
  }

  void GraphicsContextRender2D::PrepareDraw(bool shadow) noexcept
  {
    _renderer.SetBlendMode(GetBlendMode());
    _renderer.SetCompositeOperator(ToCompositeOperator2D(CompositeOperation(), GetBlendMode()));
    _renderer.SetShadowsIgnoreTransforms(ShadowsIgnoreTransforms());
    if (shadow && HasDropShadow())
    {
      const auto &dropShadow = *DropShadow();
      _renderer.SetDropShadow({.Offset = {dropShadow.Offset.Width(), dropShadow.Offset.Height()},
                               .BlurRadius = dropShadow.Radius,
                               .Colour = ToColour(dropShadow.Color)});
    }
    else
    {
      _renderer.ClearDropShadow();
    }
  }

  void GraphicsContextRender2D::FillShape(const gfx::Path2D &path, gfx::FillRule rule,
                                          const gfx::Paint2D &paint) noexcept
  {
    _renderer.FillPath(path, rule, paint);
  }

  void GraphicsContextRender2D::IntersectClipBounds(const RectF &rect) noexcept
  {
    _clipBounds.Intersect(GetCTM().MapRect(rect));
  }

  void GraphicsContextRender2D::EndImageClips() noexcept
  {
    // Ends the image clips made in the current state, innermost first.
    while (!_imageClips.empty() && _imageClips.back().StackSize >= StackSize())
    {
      const auto clip = _imageClips.back();
      _imageClips.pop_back();
      _renderer.SetTransform(clip.Transform);
      _renderer.SetBlendMode(BlendMode::Normal);
      _renderer.SetCompositeOperator(gfx::CompositeOperator2D::DestinationIn);
      _renderer.ClearDropShadow();
      _renderer.DrawImage(clip.Texture, clip.Destination, {.Filter = ImageFilter()});
      _renderer.EndLayer();
    }
  }
}
