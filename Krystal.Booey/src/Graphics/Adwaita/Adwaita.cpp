#include "Krystal.Booey/Graphics/Adwaita/Adwaita.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Platform/ThemeAdwaita.hpp"

namespace krys::boo::adwaita
{
  namespace
  {
    KRYS_NODISCARD float GetRectRadius(const FloatRect &rect, int32 offset) noexcept
    {
      return (std::min(rect.Width(), rect.Height()) + offset) / 2;
    }
  }

  void PaintFocus(GraphicsContext &graphicsContext, const FloatRect &rect, int32 offset, const Color &color,
                  PaintRounded rounded) noexcept
  {
    FloatRect focusRect = rect;
    focusRect.Inflate(static_cast<float>(offset));
    float radius = (rounded == PaintRounded::Yes) ? GetRectRadius(rect, offset) : 2;
    Path path;
    path.AddRoundedRect(focusRect, {radius, radius});
    PaintFocus(graphicsContext, path, color);
  }

  void PaintFocus(GraphicsContext &graphicsContext, const Path &path, const Color &color) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    graphicsContext.BeginTransparencyLayer(color.AlphaAsFloat());
    // Since we cut off a half of it by erasing the rect contents, and half
    // of the stroke ends up inside that area, it needs to be twice as thick.
    graphicsContext.SetStrokeThickness(FocusLineWidth * 2);
    graphicsContext.SetLineCap(LineCap::Round);
    graphicsContext.SetLineJoin(LineJoin::Round);
    graphicsContext.SetStrokeColor(color.OpaqueColor());
    graphicsContext.StrokePath(path);
    graphicsContext.SetFillRule(WindRule::NonZero);
    graphicsContext.SetCompositeOperation(CompositeOperator::Clear);
    graphicsContext.FillPath(path);
    graphicsContext.SetCompositeOperation(CompositeOperator::SourceOver);
    graphicsContext.EndTransparencyLayer();
  }

  void PaintFocus(GraphicsContext &graphicsContext, Span<const FloatRect> rects, const Color &color,
                  PaintRounded rounded) noexcept
  {
    Path path;
    for (const auto &rect : rects)
    {
      float radius = (rounded == PaintRounded::Yes) ? GetRectRadius(rect, 0) : 2;
      path.AddRoundedRect(rect, {radius, radius});
    }
    PaintFocus(graphicsContext, path, color);
  }

  void PaintArrow(GraphicsContext &graphicsContext, const FloatRect &rect, ArrowDirection direction,
                  bool useDarkAppearance) noexcept
  {
    auto offset = rect.Location();
    float size;
    if (rect.Width() > rect.Height())
    {
      size = rect.Height();
      offset.Move((rect.Width() - size) / 2, 0);
    }
    else
    {
      size = rect.Width();
      offset.Move(0, (rect.Height() - size) / 2);
    }
    float zoomFactor = size / ArrowSize;
    auto transform = [&](FloatPoint point)
    {
      point.Scale(zoomFactor);
      point.MoveBy(offset);
      return point;
    };
    Path path;
    switch (direction)
    {
      case ArrowDirection::Down:
        path.MoveTo(transform({3, 6}));
        path.AddLineTo(transform({13, 6}));
        path.AddLineTo(transform({8, 11}));
        break;
      case ArrowDirection::Up:
        path.MoveTo(transform({3, 10}));
        path.AddLineTo(transform({8, 5}));
        path.AddLineTo(transform({13, 10}));
        break;
    }
    path.CloseSubpath();

    graphicsContext.SetFillColor(useDarkAppearance ? ArrowColorDark : ArrowColorLight);
    graphicsContext.FillPath(path);
  }

  Color SystemAccentColor() noexcept
  {
    return static_cast<ThemeAdwaita &>(Theme::Singleton()).AccentColor();
  }

  Color SystemFocusRingColor() noexcept
  {
    return FocusColor(SystemAccentColor());
  }
}
