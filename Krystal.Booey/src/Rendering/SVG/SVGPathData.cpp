#include "Krystal.Booey/Rendering/SVG/SVGPathData.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"
#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "Krystal.Booey/SVG/SVGPolygonElement.hpp"
#include "Krystal.Booey/SVG/SVGPolylineElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"

namespace krys::boo
{
  namespace
  {
    Path PathFromCircleElement(const svg::SVGCircleElement &element) noexcept
    {
      RenderElement *renderer = element.Renderer();
      if (!renderer)
      {
        return {};
      }

      Path path;
      const auto &style = renderer->Style();
      svg::SVGLengthContext lengthContext(&element);
      float r = lengthContext.ValueForLength(style.R(), style::ZoomNeeded {});
      if (r > 0)
      {
        float cx = lengthContext.ValueForLength(style.Cx(), style::ZoomNeeded {}, svg::SVGLengthMode::Width);
        float cy = lengthContext.ValueForLength(style.Cy(), style::ZoomNeeded {}, svg::SVGLengthMode::Height);
        path.AddEllipseInRect(FloatRect(cx - r, cy - r, r * 2, r * 2));
      }
      return path;
    }

    Path PathFromEllipseElement(const svg::SVGEllipseElement &element) noexcept
    {
      RenderElement *renderer = element.Renderer();
      if (!renderer)
      {
        return {};
      }

      const auto &style = renderer->Style();
      svg::SVGLengthContext lengthContext(&element);
      float rx = lengthContext.ValueForLength(style.Rx(), style::ZoomNeeded {}, svg::SVGLengthMode::Width);
      if (rx <= 0)
      {
        return {};
      }

      float ry = lengthContext.ValueForLength(style.Ry(), style::ZoomNeeded {}, svg::SVGLengthMode::Height);
      if (ry <= 0)
      {
        return {};
      }

      Path path;
      float cx = lengthContext.ValueForLength(style.Cx(), style::ZoomNeeded {}, svg::SVGLengthMode::Width);
      float cy = lengthContext.ValueForLength(style.Cy(), style::ZoomNeeded {}, svg::SVGLengthMode::Height);
      path.AddEllipseInRect(FloatRect(cx - rx, cy - ry, rx * 2, ry * 2));
      return path;
    }

    Path PathFromLineElement(const svg::SVGLineElement &element) noexcept
    {
      Path path;
      svg::SVGLengthContext lengthContext(&element);
      path.MoveTo(FloatPoint(element.X1().Value(lengthContext), element.Y1().Value(lengthContext)));
      path.AddLineTo(FloatPoint(element.X2().Value(lengthContext), element.Y2().Value(lengthContext)));
      return path;
    }

    Path PathFromPathElement(const svg::SVGPathElement &element) noexcept
    {
      return element.GetPath();
    }

    Path PathFromPolygonElement(const svg::SVGPolygonElement &element) noexcept
    {
      const auto &points = element.Points().Items();
      if (points.empty())
      {
        return {};
      }

      Path path;
      path.MoveTo(points.front()->Value());

      size_t size = points.size();
      for (size_t i = 1; i < size; ++i)
      {
        path.AddLineTo(points[i]->Value());
      }

      path.CloseSubpath();
      return path;
    }

    Path PathFromPolylineElement(const svg::SVGPolylineElement &element) noexcept
    {
      const auto &points = element.Points().Items();
      if (points.empty())
      {
        return {};
      }

      Path path;
      path.MoveTo(points.front()->Value());

      size_t size = points.size();
      for (size_t i = 1; i < size; ++i)
      {
        path.AddLineTo(points[i]->Value());
      }
      return path;
    }

    Path PathFromRectElement(const svg::SVGRectElement &element) noexcept
    {
      RenderElement *renderer = element.Renderer();
      if (!renderer)
      {
        return {};
      }

      const auto &style = renderer->Style();
      auto usedZoom = style.UsedZoomForLength();
      svg::SVGLengthContext lengthContext(&element);
      auto size =
        FloatSize {lengthContext.ValueForLength(style.Width(), usedZoom, svg::SVGLengthMode::Width),
                   lengthContext.ValueForLength(style.Height(), usedZoom, svg::SVGLengthMode::Height)};

      if (size.IsEmpty())
      {
        return {};
      }

      auto location = FloatPoint {
        lengthContext.ValueForLength(style.X(), style::ZoomNeeded {}, svg::SVGLengthMode::Width),
        lengthContext.ValueForLength(style.Y(), style::ZoomNeeded {}, svg::SVGLengthMode::Height)};

      auto radii = FloatSize {
        lengthContext.ValueForLength(style.Rx(), style::ZoomNeeded {}, svg::SVGLengthMode::Width),
        lengthContext.ValueForLength(style.Ry(), style::ZoomNeeded {}, svg::SVGLengthMode::Height)};

      // Per SVG spec: if one of radii.x() and radii.y() is auto or negative, then the other corner
      // radius value is used. If both are auto or negative, then they are both set to 0.
      if (style.Rx().IsAuto() || radii.Width() < 0)
      {
        radii.SetWidth(std::max(0.f, radii.Height()));
      }
      if (style.Ry().IsAuto() || radii.Height() < 0)
      {
        radii.SetHeight(std::max(0.f, radii.Width()));
      }

      Path path;
      if (radii.Width() <= 0 && radii.Height() <= 0)
      {
        path.AddRect({location, size});
        return path;
      }

      // The used values of 'rx' and 'ry' for a 'rect' are never more than 50% of the used
      // value of width for the same shape.
      radii = radii.ConstrainedBetween(radii, size / 2);

      // FIXME: We currently enforce using beziers here, as at least on CoreGraphics/Lion, as
      // the native method uses a different line dash origin, causing svg/custom/dashOrigin.svg to fail.
      // See bug https://bugs.webkit.org/show_bug.cgi?id=79932 which tracks this issue.
      path.AddRoundedRect(FloatRect {location, size}, radii, PathRoundedRect::Strategy::PreferBezier);
      return path;
    }
  }

  namespace
  {
    KRYS_NODISCARD Path PathFromUseElement(const svg::SVGUseElement &element) noexcept
    {
      RefPtr<svg::SVGElement> clipChildElement = element.ClipChild();
      if (!clipChildElement)
      {
        return {};
      }

      svg::SVGLengthContext lengthContext(&element);
      auto x = element.X().Value(lengthContext);
      auto y = element.Y().Value(lengthContext);

      auto path = PathFromGraphicsElement(*clipChildElement);
      if (x || y)
      {
        path.Translate(FloatSize(x, y));
      }

      return path;
    }
  }

  Path PathFromGraphicsElement(const svg::SVGElement &element) noexcept
  {
    if (auto *circle = DynamicDowncast<svg::SVGCircleElement>(element))
    {
      return PathFromCircleElement(*circle);
    }
    if (auto *ellipse = DynamicDowncast<svg::SVGEllipseElement>(element))
    {
      return PathFromEllipseElement(*ellipse);
    }
    if (auto *line = DynamicDowncast<svg::SVGLineElement>(element))
    {
      return PathFromLineElement(*line);
    }
    if (auto *path = DynamicDowncast<svg::SVGPathElement>(element))
    {
      return PathFromPathElement(*path);
    }
    if (auto *polygon = DynamicDowncast<svg::SVGPolygonElement>(element))
    {
      return PathFromPolygonElement(*polygon);
    }
    if (auto *polyline = DynamicDowncast<svg::SVGPolylineElement>(element))
    {
      return PathFromPolylineElement(*polyline);
    }
    if (auto *rect = DynamicDowncast<svg::SVGRectElement>(element))
    {
      return PathFromRectElement(*rect);
    }
    if (auto *use = DynamicDowncast<svg::SVGUseElement>(element))
    {
      return PathFromUseElement(*use);
    }

    return {};
  }
}
