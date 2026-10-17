#include "Krystal.Booey/Rendering/Shapes/BoxLayoutShape.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD LayoutUnit AdjustRadiusForMarginBoxShape(LayoutUnit radius, LayoutUnit margin) noexcept
    {
      // This algorithm is defined in the CSS Shapes specifcation
      if (!margin)
      {
        return radius;
      }

      LayoutUnit ratio = radius / margin;
      if (ratio < 1)
      {
        return LayoutUnit(radius.ToDouble()
                          + (margin.ToDouble() * (1 + std::pow(ratio.ToDouble() - 1, 3.0))));
      }

      return radius + margin;
    }

    KRYS_NODISCARD LayoutSize ComputeMarginBoxShapeRadius(const LayoutSize &radius,
                                                          const LayoutSize &adjacentMargins) noexcept
    {
      return LayoutSize(AdjustRadiusForMarginBoxShape(radius.Width(), adjacentMargins.Width()),
                        AdjustRadiusForMarginBoxShape(radius.Height(), adjacentMargins.Height()));
    }

    KRYS_NODISCARD LayoutRoundedRect::Radii ComputeMarginBoxShapeRadii(const LayoutRoundedRect::Radii &radii,
                                                                       const RenderBox &renderer) noexcept
    {
      return LayoutRoundedRect::Radii(
        ComputeMarginBoxShapeRadius(radii.TopLeft(), LayoutSize(renderer.MarginLeft(), renderer.MarginTop())),
        ComputeMarginBoxShapeRadius(radii.TopRight(),
                                    LayoutSize(renderer.MarginRight(), renderer.MarginTop())),
        ComputeMarginBoxShapeRadius(radii.BottomLeft(),
                                    LayoutSize(renderer.MarginLeft(), renderer.MarginBottom())),
        ComputeMarginBoxShapeRadius(radii.BottomRight(),
                                    LayoutSize(renderer.MarginRight(), renderer.MarginBottom())));
    }
  }

  LayoutRoundedRect ComputeRoundedRectForBoxShape(CSSBoxType box, const RenderBox &renderer) noexcept
  {
    const RenderStyle &style = renderer.Style();
    switch (box)
    {
      case CSSBoxType::MarginBox:
      {
        if (!style.HasBorderRadius())
        {
          return LayoutRoundedRect(renderer.MarginBoxRect(), LayoutRoundedRect::Radii());
        }

        auto marginBox = renderer.MarginBoxRect();
        auto borderShape = BorderShape::ShapeForBorderRect(style, renderer.BorderBoxRect());
        LayoutRoundedRect::Radii radii = ComputeMarginBoxShapeRadii(borderShape.Radii(), renderer);
        radii.Scale(CalcBorderRadiiConstraintScaleFor(marginBox, radii));
        return LayoutRoundedRect(marginBox, radii);
      }
      case CSSBoxType::PaddingBox:
        return BorderShape::ShapeForBorderRect(style, renderer.BorderBoxRect()).DeprecatedInnerRoundedRect();
      // fill-box compute to content-box for HTML elements.
      case CSSBoxType::FillBox:
      case CSSBoxType::ContentBox:
      {
        auto borderShape = renderer.BorderShapeForContentClipping(renderer.BorderBoxRect());
        return borderShape.DeprecatedInnerRoundedRect();
      }
      // stroke-box, view-box compute to border-box for HTML elements.
      case CSSBoxType::BorderBox:
      case CSSBoxType::StrokeBox:
      case CSSBoxType::ViewBox:
      case CSSBoxType::BoxMissing:
        return BorderShape::ShapeForBorderRect(style, renderer.BorderBoxRect()).DeprecatedRoundedRect();
    }

    krys_debug_assert(false);
    return BorderShape::ShapeForBorderRect(style, renderer.BorderBoxRect()).DeprecatedRoundedRect();
  }

  LayoutRect BoxLayoutShape::ShapeMarginLogicalBoundingBox() const noexcept
  {
    FloatRect marginBounds(_bounds.Rect());
    if (ShapeMargin() > 0)
    {
      marginBounds.Inflate(ShapeMargin());
    }
    return LayoutRect(marginBounds);
  }

  FloatRoundedRect BoxLayoutShape::ShapeMarginBounds() const noexcept
  {
    auto shapeMargin = this->ShapeMargin();
    if (!shapeMargin)
    {
      return _bounds;
    }

    auto marginBounds = FloatRoundedRect {_bounds};
    marginBounds.Inflate(shapeMargin);
    auto expandedRadii = marginBounds.GetRadii();
    expandedRadii.ExpandEvenIfZero(shapeMargin);
    marginBounds.SetRadii(expandedRadii);
    return marginBounds;
  }

  LineSegment BoxLayoutShape::GetExcludedInterval(LayoutUnit logicalTop,
                                                  LayoutUnit logicalHeight) const noexcept
  {
    const FloatRoundedRect &marginBounds = ShapeMarginBounds();
    if (marginBounds.IsEmpty() || !LineOverlapsShapeMarginBounds(logicalTop, logicalHeight))
    {
      return LineSegment();
    }

    float y1 = logicalTop.ToFloat();
    float y2 = (logicalTop + logicalHeight).ToFloat();
    const FloatRect &rect = marginBounds.Rect();

    if (!marginBounds.IsRounded())
    {
      return LineSegment(rect.X(), rect.MaxX());
    }

    float topCornerMaxY =
      std::max<float>(marginBounds.TopLeftCorner().MaxY(), marginBounds.TopRightCorner().MaxY());
    float bottomCornerMinY =
      std::min<float>(marginBounds.BottomLeftCorner().Y(), marginBounds.BottomRightCorner().Y());

    if (topCornerMaxY <= bottomCornerMinY && y1 <= topCornerMaxY && y2 >= bottomCornerMinY)
    {
      return LineSegment(rect.X(), rect.MaxX());
    }

    float x1 = rect.MaxX();
    float x2 = rect.X();
    float minXIntercept;
    float maxXIntercept;

    if (y1 <= marginBounds.TopLeftCorner().MaxY() && y2 >= marginBounds.BottomLeftCorner().Y())
    {
      x1 = rect.X();
    }

    if (y1 <= marginBounds.TopRightCorner().MaxY() && y2 >= marginBounds.BottomRightCorner().Y())
    {
      x2 = rect.MaxX();
    }

    if (marginBounds.XInterceptsAtY(y1, minXIntercept, maxXIntercept))
    {
      x1 = std::min<float>(x1, minXIntercept);
      x2 = std::max<float>(x2, maxXIntercept);
    }

    if (marginBounds.XInterceptsAtY(y2, minXIntercept, maxXIntercept))
    {
      x1 = std::min<float>(x1, minXIntercept);
      x2 = std::max<float>(x2, maxXIntercept);
    }

    krys_debug_assert(x2 >= x1);
    return LineSegment(x1, x2);
  }
}
