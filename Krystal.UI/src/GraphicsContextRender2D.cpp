#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.UI/FilterGraph2DBuilder.hpp"
#include "Krystal.UI/ImageBufferTextures.hpp"
#include "Krystal.UI/Render2DConversions.hpp"
#include <cmath>
#include <limits>
#include <numbers>

namespace krys::ui
{
  namespace
  {
    KRYS_NODISCARD boo::AffineTransform ToAffineTransform(const gfx::Transform2D &transform) noexcept
    {
      return {transform.A, transform.B, transform.C, transform.D, transform.E, transform.F};
    }

    /// @brief The operator Render2D draws @p op with, blending with @p blendMode as Skia does: a blend mode
    /// other than normal blends over. Plus darker and difference aren't Porter-Duff operators: they draw
    /// over.
    KRYS_NODISCARD gfx::CompositeOperator2D ToCompositeOperator2D(boo::CompositeOperator op,
                                                                  boo::BlendMode blendMode) noexcept
    {
      using Op = gfx::CompositeOperator2D;
      if (blendMode != boo::BlendMode::Normal)
      {
        return Op::SourceOver;
      }
      switch (op)
      {
        case boo::CompositeOperator::Clear:           return Op::Clear;
        case boo::CompositeOperator::Copy:            return Op::Copy;
        case boo::CompositeOperator::SourceIn:        return Op::SourceIn;
        case boo::CompositeOperator::SourceOut:       return Op::SourceOut;
        case boo::CompositeOperator::SourceAtop:      return Op::SourceAtop;
        case boo::CompositeOperator::DestinationOver: return Op::DestinationOver;
        case boo::CompositeOperator::DestinationIn:   return Op::DestinationIn;
        case boo::CompositeOperator::DestinationOut:  return Op::DestinationOut;
        case boo::CompositeOperator::DestinationAtop: return Op::DestinationAtop;
        case boo::CompositeOperator::XOR:             return Op::Xor;
        case boo::CompositeOperator::PlusLighter:     return Op::PlusLighter;
        // FIXME: Plus darker and difference as compositing operators, once Render2D has them.
        default:                                      return Op::SourceOver;
      }
    }

    KRYS_NODISCARD bool IsValidDashArray(const boo::DashArray &dashArray) noexcept
    {
      // See 'dom-context-2d-setlinedash': if the array contains not finite or negative values, return.
      boo::DashArrayElement total = 0;
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
    KRYS_NODISCARD gfx::Path2D CreateErrorUnderlinePath(const boo::FloatRect &boundaries) noexcept
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
                                                   const boo::FloatRect &bounds,
                                                   ImageBufferTextures *imageBuffers) noexcept
      : _renderer(renderer), _fonts(fonts), _imageBuffers(imageBuffers), _clipBounds(bounds)
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

  void GraphicsContextRender2D::Save(boo::GraphicsContextState::Purpose purpose) noexcept
  {
    GraphicsContext::Save(purpose);
    _strokeStack.push_back(_stroke);
    _clipBoundsStack.push_back(_clipBounds);
    _renderer.Save();
  }

  void GraphicsContextRender2D::Restore(boo::GraphicsContextState::Purpose purpose) noexcept
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
  void GraphicsContextRender2D::DrawRect(const boo::FloatRect &rect, float borderThickness) noexcept
  {
    krys_debug_assert(!rect.IsEmpty());
    PrepareDraw();
    _renderer.FillRect(ToRect2D(rect), FillBrushPaint().Paint());
    if (GetStrokeStyle() == boo::StrokeStyle::NoStroke)
    {
      return;
    }

    const boo::FloatRect rects[4] = {
      {rect.X(), rect.Y(), rect.Width(), borderThickness},
      {rect.X(), rect.MaxY() - borderThickness, rect.Width(), borderThickness},
      {rect.X(), rect.Y() + borderThickness, borderThickness, rect.Height() - 2 * borderThickness},
      {rect.MaxX() - borderThickness, rect.Y() + borderThickness, borderThickness,
       rect.Height() - 2 * borderThickness}};
    const auto stroke = StrokeBrushPaint();
    for (const auto &side : rects)
    {
      _renderer.FillRect(ToRect2D(side), stroke.Paint());
    }
  }

  // This is only used to draw borders, so we should not draw shadows.
  void GraphicsContextRender2D::DrawLine(const boo::FloatPoint &point1,
                                         const boo::FloatPoint &point2) noexcept
  {
    if (GetStrokeStyle() == boo::StrokeStyle::NoStroke)
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

    if (GetStrokeStyle() == boo::StrokeStyle::DottedStroke
        || GetStrokeStyle() == boo::StrokeStyle::DashedStroke)
    {
      // Figure out end points to ensure we always paint corners.
      cornerWidth = DashedLineCornerWidthForStrokeWidth(strokeWidth);
      if (isVertical)
      {
        FillRect(boo::FloatRect(point1.X(), point1.Y(), StrokeThickness(), cornerWidth), StrokeColor());
        FillRect(boo::FloatRect(point1.X(), point2.Y() - cornerWidth, StrokeThickness(), cornerWidth),
                 StrokeColor());
      }
      else
      {
        FillRect(boo::FloatRect(point1.X(), point1.Y(), cornerWidth, StrokeThickness()), StrokeColor());
        FillRect(boo::FloatRect(point2.X() - cornerWidth, point1.Y(), cornerWidth, StrokeThickness()),
                 StrokeColor());
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
  void GraphicsContextRender2D::DrawEllipse(const boo::FloatRect &boundaries) noexcept
  {
    gfx::Path2D path;
    path.AddEllipse(ToVec2f(boundaries.Center()), {boundaries.Width() / 2.f, boundaries.Height() / 2.f});
    PrepareDraw();
    FillShape(path, gfx::FillRule::NonZero, FillBrushPaint().Paint());
  }

  void GraphicsContextRender2D::FillPath(const boo::Path &path) noexcept
  {
    if (path.IsEmpty())
    {
      return;
    }

    PrepareDraw();
    FillShape(ToPath2D(path), ToFillRule(FillRule()), FillBrushPaint().Paint());
  }

  void GraphicsContextRender2D::StrokePath(const boo::Path &path) noexcept
  {
    if (path.IsEmpty())
    {
      return;
    }

    PrepareDraw();
    _renderer.StrokePath(ToPath2D(path), CurrentStrokeStyle(StrokeThickness()), StrokeBrushPaint().Paint());
  }

  void GraphicsContextRender2D::FillRect(const boo::FloatRect &boundaries, RequiresClipToRect) noexcept
  {
    PrepareDraw();
    _renderer.FillRect(ToRect2D(boundaries), FillBrushPaint().Paint());
  }

  void GraphicsContextRender2D::FillRect(const boo::FloatRect &boundaries,
                                         const boo::Color &fillColor) noexcept
  {
    PrepareDraw();
    _renderer.FillRect(ToRect2D(boundaries), ToColour(fillColor));
  }

  void GraphicsContextRender2D::FillRect(const boo::FloatRect &boundaries, boo::Gradient &gradient,
                                         const boo::AffineTransform &gradientSpaceTransform,
                                         RequiresClipToRect) noexcept
  {
    PrepareDraw();
    const auto gradient2D = ToGradient2D(gradient, Alpha());
    _renderer.FillRect(ToRect2D(boundaries), gfx::Paint2D(gradient2D, ToTransform2D(gradientSpaceTransform)));
  }

  void GraphicsContextRender2D::FillRoundedRectImpl(const boo::FloatRoundedRect &rect,
                                                    const boo::Color &color) noexcept
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

  void GraphicsContextRender2D::FillRectWithRoundedHole(const boo::FloatRect &outerRect,
                                                        const boo::FloatRoundedRect &innerRRect,
                                                        const boo::Color &color) noexcept
  {
    if (!color.IsValid())
    {
      return;
    }

    auto fill = [&]
    {
      if (auto circular = ToCircularRoundedRect(innerRRect))
      {
        _renderer.FillRectWithRoundedHole(ToRect2D(outerRect), *circular, ToColour(color));
        return;
      }

      gfx::Path2D path;
      path.AddRect(ToRect2D(outerRect));
      AddRoundedRect(path, innerRRect);
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
    else if (auto inverse = ToAffineTransform(transform).Inverse())
    {
      shadow.Offset = shadow.Offset - inverse->MapSize(boo::FloatSize {away, 0});
    }
    _renderer.Save();
    _renderer.SetTransform(gfx::Transform2D::Translation(away, 0.f) * transform);
    _renderer.SetDropShadow({.Offset = {shadow.Offset.Width(), shadow.Offset.Height()},
                             .BlurRadius = shadow.Radius,
                             .Colour = ToColour(shadow.Color)});
    fill();
    _renderer.Restore();
  }

  void GraphicsContextRender2D::ClearRect(const boo::FloatRect &) noexcept
  {
    // FIXME: Clearing needs the clear compositing operator.
  }

  void GraphicsContextRender2D::StrokeRect(const boo::FloatRect &boundaries, float lineWidth) noexcept
  {
    gfx::Path2D path;
    path.AddRect(ToRect2D(boundaries));
    PrepareDraw();
    _renderer.StrokePath(path, CurrentStrokeStyle(lineWidth), StrokeBrushPaint().Paint());
  }

  void GraphicsContextRender2D::SetLineCap(boo::LineCap lineCap) noexcept
  {
    _stroke.Cap = ToLineCap(lineCap);
  }

  void GraphicsContextRender2D::SetLineDash(const boo::DashArray &dashArray, float dashOffset) noexcept
  {
    _stroke.Dashes.clear();
    _stroke.DashOffset = 0.f;
    if (!IsValidDashArray(dashArray))
    {
      return;
    }

    // Repeat the array to ensure even number of dash array elements, see e.g. 'stroke-dasharray' spec.
    const size_t count = dashArray.size() % 2 == 1 ? dashArray.size() * 2 : dashArray.size();
    for (size_t i = 0; i < count; ++i)
    {
      _stroke.Dashes.push_back(dashArray[i % dashArray.size()]);
    }
    _stroke.DashOffset = dashOffset;
  }

  void GraphicsContextRender2D::SetLineJoin(boo::LineJoin lineJoin) noexcept
  {
    _stroke.Join = ToLineJoin(lineJoin);
  }

  void GraphicsContextRender2D::SetMiterLimit(float miter) noexcept
  {
    _stroke.MiterLimit = miter;
  }

  void GraphicsContextRender2D::ResetClip() noexcept
  {
    // FIXME: Render2D can't reset its clip.
  }

  void GraphicsContextRender2D::Clip(const boo::FloatRect &rect) noexcept
  {
    IntersectClipBounds(rect);
    _renderer.ClipRect(ToRect2D(rect));
  }

  void GraphicsContextRender2D::ClipRoundedRect(const boo::FloatRoundedRect &rect) noexcept
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

  void GraphicsContextRender2D::ClipOut(const boo::FloatRect &rect) noexcept
  {
    _renderer.ClipOutRect(ToRect2D(rect));
  }

  void GraphicsContextRender2D::ClipOut(const boo::Path &path) noexcept
  {
    _renderer.ClipOutPath(ToPath2D(path), gfx::FillRule::NonZero);
  }

  void GraphicsContextRender2D::ClipOutRoundedRect(const boo::FloatRoundedRect &rect) noexcept
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

  void GraphicsContextRender2D::ClipPath(const boo::Path &path, boo::WindRule clipRule) noexcept
  {
    IntersectClipBounds(path.FastBoundingRect());
    _renderer.ClipPath(ToPath2D(path), ToFillRule(clipRule));
  }

  void GraphicsContextRender2D::ClipToImageBuffer(boo::ImageBuffer &image,
                                                  const boo::FloatRect &destination) noexcept
  {
    // As Skia's clip shader, which leaves nothing outside the image. Render2D can't clip to a texture, so
    // what's drawn until this state ends goes into a layer, which the image's alpha masks as it ends.
    IntersectClipBounds(destination);
    _renderer.ClipRect(ToRect2D(destination));

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
                           .Destination = ToRect2D({destination.Location(), image.LogicalSize()}),
                           .Transform = _renderer.GetTransform(),
                           .StackSize = StackSize()});
    _renderer.BeginLayer(1.f);
  }

  boo::IntRect GraphicsContextRender2D::ClipBounds() const noexcept
  {
    if (auto inverse = GetCTM().Inverse())
    {
      return boo::EnclosingIntRect(inverse->MapRect(_clipBounds));
    }
    return boo::EnclosingIntRect(_clipBounds);
  }

  void GraphicsContextRender2D::DrawGlyphs(const boo::Font &font, Span<const boo::GlyphBufferGlyph> glyphs,
                                           Span<const boo::GlyphBufferAdvance> advances,
                                           const boo::FloatPoint &position, boo::FontSmoothingMode) noexcept
  {
    if (!GetTextDrawingMode().Contains(boo::TextDrawingMode::Fill) || glyphs.empty())
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

    const bool isVertical = font.PlatformData().Orientation() == boo::FontOrientation::Vertical;
    if (isVertical)
    {
      _renderer.Save();
      _renderer.Translate(position.X(), position.Y());
      _renderer.Rotate(-std::numbers::pi_v<float> / 2.f);
      _renderer.Translate(-position.X(), -position.Y());
    }

    PrepareDraw();
    _renderer.DrawGlyphs(*face, font.PlatformData().Size(), glyphIndices, positions, ToVec2f(position),
                         FillBrushPaint().Paint());

    if (isVertical)
    {
      _renderer.Restore();
    }
  }

  void GraphicsContextRender2D::DrawLinesForText(const boo::FloatPoint &point, float thickness,
                                                 Span<const boo::FloatSegment> lineSegments, bool printing,
                                                 bool doubleUnderlines, boo::StrokeStyle strokeStyle) noexcept
  {
    auto [rects, strokeColor] = ComputeRectsAndStrokeColorForLinesForText(
      point, thickness, lineSegments, printing, doubleUnderlines, strokeStyle);
    for (auto &rect : rects)
    {
      FillRect(rect, strokeColor);
    }
  }

  void GraphicsContextRender2D::DrawDotsForDocumentMarker(const boo::FloatRect &boundaries,
                                                          boo::DocumentMarkerLineStyle style) noexcept
  {
    if (style.Mode != boo::DocumentMarkerLineStyleMode::Spelling
        && style.Mode != boo::DocumentMarkerLineStyleMode::Grammar)
    {
      return;
    }

    PrepareDraw(false);
    _renderer.FillPath(CreateErrorUnderlinePath(boundaries), gfx::FillRule::NonZero, ToColour(style.Color));
  }

  void GraphicsContextRender2D::BeginTransparencyLayer(float opacity) noexcept
  {
    GraphicsContext::BeginTransparencyLayer(opacity);
    _layerStack.push_back({});
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
      [](const boo::GraphicsColorMatrix &matrix) { return gfx::LayerFilter2D::ColourMatrix(matrix.Values); },
      [&](const boo::GraphicsGaussianBlur &blur)
      {
        // A layer blurs evenly: an uneven blur uses the larger radius.
        return gfx::LayerFilter2D::Blur(std::max(blur.Radius.Width(), blur.Radius.Height()) * scale);
      },
      [&](const boo::GraphicsDropShadow &shadow)
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

  void GraphicsContextRender2D::BeginTransparencyLayer(boo::CompositeOperator operation,
                                                       boo::BlendMode blendMode) noexcept
  {
    GraphicsContext::BeginTransparencyLayer(operation, blendMode);
    _layerStack.push_back({boo::CompositeMode {operation, blendMode}});
    _clipBoundsStack.push_back(_clipBounds);
    _renderer.SetCompositeOperator(ToCompositeOperator2D(operation, blendMode));
    _renderer.BeginLayer(1.f, blendMode);
    // When on transparency layer, we don't want to blend operations as when layer ends, we blend it as a
    // whole.
    SetCompositeMode({boo::CompositeOperator::SourceOver, BlendMode::Normal});
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

  void GraphicsContextRender2D::DrawFocusRing(const boo::Path &path, float, const boo::Color &color) noexcept
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

  void GraphicsContextRender2D::DrawFocusRing(const List<boo::FloatRect> &rects, float, float outlineWidth,
                                              const boo::Color &color) noexcept
  {
    // Adwaita rounds each rect's corners by 2.
    boo::Path path;
    for (const auto &rect : rects)
    {
      path.AddRoundedRect(rect, boo::FloatSize(2, 2));
    }
    DrawFocusRing(path, outlineWidth, color);
  }

  void GraphicsContextRender2D::Scale(const boo::FloatSize &scale) noexcept
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

  void GraphicsContextRender2D::ConcatCTM(const boo::AffineTransform &transform) noexcept
  {
    _renderer.ConcatTransform(ToTransform2D(transform));
  }

  void GraphicsContextRender2D::SetCTM(const boo::AffineTransform &transform) noexcept
  {
    _renderer.SetTransform(ToTransform2D(transform));
  }

  boo::AffineTransform GraphicsContextRender2D::GetCTM(IncludeDeviceScale) const noexcept
  {
    return ToAffineTransform(_renderer.GetTransform());
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
      return {.Gradient = ToGradient2D(*gradient, Alpha()),
              .GradientSpace = ToTransform2D(brush.GradientSpaceTransform())};
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
      return {.Gradient = ToGradient2D(*gradient, Alpha()),
              .GradientSpace = ToTransform2D(brush.GradientSpaceTransform())};
    }
    return {.Colour = ToColour(brush.GetColor(), Alpha())};
  }

  GraphicsContextRender2D::Brush
    GraphicsContextRender2D::PatternBrushPaint(const boo::Pattern &pattern) const noexcept
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
              .PatternSpace = ToTransform2D(pattern.PatternSpaceTransform()),
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
    return ImageInterpolationQuality() == boo::InterpolationQuality::DoNotInterpolate
             ? gfx::SamplerFilter::Nearest
             : gfx::SamplerFilter::Linear;
  }

  void GraphicsContextRender2D::DrawNativeImage(boo::NativeImage &image, const boo::FloatRect &destRect,
                                                const boo::FloatRect &srcRect,
                                                boo::ImagePaintingOptions options) noexcept
  {
    auto *commands = _renderer.GetCommandBuffer();
    if (!_imageBuffers || !commands)
    {
      return;
    }

    auto orientation = options.Orientation();
    auto imageSize = boo::FloatSize(image.Size());
    if (orientation.UsesWidthAsHeight())
    {
      imageSize = imageSize.TransposedSize();
    }
    auto normalizedSrcRect = boo::NormaliseRect(srcRect);
    if (!boo::FloatRect({}, imageSize).Intersects(normalizedSrcRect))
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

    auto normalizedDestRect = boo::NormaliseRect(destRect);
    boo::GraphicsContextStateSaver stateSaver(*this, orientation != boo::ImageOrientation::Orientation::None);
    if (orientation != boo::ImageOrientation::Orientation::None)
    {
      // ImageOrientation expects the origin to be at (0, 0).
      Translate(normalizedDestRect.X(), normalizedDestRect.Y());
      normalizedDestRect.SetLocation(boo::FloatPoint());
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

  void GraphicsContextRender2D::DrawPattern(boo::NativeImage &image, const boo::FloatRect &destRect,
                                            const boo::FloatRect &tileRect,
                                            const boo::AffineTransform &patternTransform,
                                            const boo::FloatPoint &phase, const boo::FloatSize &spacing,
                                            boo::ImagePaintingOptions options) noexcept
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
    DrawTexturePattern(texture, boo::FloatSize(image.Size()), destRect, tileRect, patternTransform, phase,
                       spacing, options, *commands);
  }

  void GraphicsContextRender2D::DrawImageBuffer(boo::ImageBuffer &image, const boo::FloatRect &destination,
                                                const boo::FloatRect &source,
                                                boo::ImagePaintingOptions options) noexcept
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

    boo::FloatRect sourceScaled = source;
    sourceScaled.Scale(image.ResolutionScale());
    DrawTexture(texture, sourceScaled, destination, options);
  }

  void GraphicsContextRender2D::DrawFilteredImageBuffer(boo::ImageBuffer *sourceImage,
                                                        const boo::FloatRect &sourceImageRect,
                                                        boo::Filter &filter, boo::FilterResults &) noexcept
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
    boo::FilterResults results([&](const boo::FilterEffect &effect)
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
    const boo::FloatRect source(0.f, 0.f, static_cast<float>(region.Width),
                                static_cast<float>(region.Height));
    Scale({1 / filter.FilterScale().Width(), 1 / filter.FilterScale().Height()});
    DrawTexture(rendered->Texture, source, boo::FloatRect(result->AbsoluteImageRect()), {});
    Scale(filter.FilterScale());
  }

  void GraphicsContextRender2D::DrawPattern(boo::ImageBuffer &image, const boo::FloatRect &destRect,
                                            const boo::FloatRect &tileRect,
                                            const boo::AffineTransform &patternTransform,
                                            const boo::FloatPoint &phase, const boo::FloatSize &spacing,
                                            boo::ImagePaintingOptions options) noexcept
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
    DrawTexturePattern(texture, boo::FloatSize(image.BackendSize()), destRect, tileRect, patternTransform,
                       phase, spacing, options, *commands);
  }

  void GraphicsContextRender2D::DrawTexture(gfx::TextureHandle texture, const boo::FloatRect &source,
                                            const boo::FloatRect &destination,
                                            boo::ImagePaintingOptions options,
                                            AlphaPremultiplication alpha) noexcept
  {
    boo::InterpolationQualityMaintainer interpolationQualityForThisScope(*this,
                                                                         options.GetInterpolationQuality());
    PrepareDraw();
    _renderer.Save();
    PrepareImageDraw(options);
    _renderer.DrawImage(texture, ToRect2D(source), ToRect2D(destination),
                        {.Filter = ImageFilter(), .Alpha = alpha, .Opacity = Alpha()});
    _renderer.Restore();
  }

  void GraphicsContextRender2D::DrawTexturePattern(
    gfx::TextureHandle texture, const boo::FloatSize &size, const boo::FloatRect &destRect,
    const boo::FloatRect &tileRect, const boo::AffineTransform &patternTransform,
    const boo::FloatPoint &phase, const boo::FloatSize &spacing, boo::ImagePaintingOptions options,
    gfx::CommandBuffer &commands) noexcept
  {
    // As Skia's: the shader places the tile at the phase, moved by the tile's place in the image under the
    // pattern transform's scale. The tile is the whole texture, so a tile that's part of the image (or has
    // spacing after it) is copied into a texture of its own, its size rounded out to whole texels.
    boo::FloatPoint phaseOffset(tileRect.Location());
    phaseOffset.Scale(static_cast<float>(patternTransform.A()), static_cast<float>(patternTransform.D()));
    phaseOffset.MoveBy(phase);
    const auto shaderMatrix =
      gfx::Transform2D::Translation(phaseOffset.X(), phaseOffset.Y()) * ToTransform2D(patternTransform);

    auto tileTexture = texture;
    bool repeatX = true;
    bool repeatY = true;
    if (spacing.IsZero() && tileRect.Size() == size)
    {
      // Repeat only along an axis the destination reaches past the image on; otherwise clamp, so a tile
      // scaled to fit the destination exactly doesn't filter in the texels at its other end.
      if (const auto inverse = shaderMatrix.Inverse())
      {
        const auto corners = {inverse->Apply({destRect.X(), destRect.Y()}),
                              inverse->Apply({destRect.MaxX(), destRect.Y()}),
                              inverse->Apply({destRect.X(), destRect.MaxY()}),
                              inverse->Apply({destRect.MaxX(), destRect.MaxY()})};
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
      const auto enclosing = boo::EnclosingIntRect(tileRect);
      tileTexture = _imageBuffers->Copy(texture, ToRect2D(tileRect), static_cast<uint32>(enclosing.Width()),
                                        static_cast<uint32>(enclosing.Height()), commands);
      if (!tileTexture.IsValid())
      {
        return;
      }
    }

    boo::InterpolationQualityMaintainer interpolationQualityForThisScope(*this,
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
    _renderer.FillRect(ToRect2D(destRect), pattern);
    _renderer.Restore();
  }

  void GraphicsContextRender2D::PrepareImageDraw(const boo::ImagePaintingOptions &options) noexcept
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

  void GraphicsContextRender2D::IntersectClipBounds(const boo::FloatRect &rect) noexcept
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
