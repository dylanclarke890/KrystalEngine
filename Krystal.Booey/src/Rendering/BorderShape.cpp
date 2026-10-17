#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Borders/BorderRadius.hpp"

namespace krys::boo
{
  namespace
  {
    void ClearRadiiOfOpenEdges(LayoutRoundedRectRadii &radii, RectEdges<bool> closedEdges) noexcept
    {
      if (!closedEdges.Top())
      {
        radii.SetTopLeft({});
        radii.SetTopRight({});
      }
      if (!closedEdges.Right())
      {
        radii.SetTopRight({});
        radii.SetBottomRight({});
      }
      if (!closedEdges.Bottom())
      {
        radii.SetBottomRight({});
        radii.SetBottomLeft({});
      }
      if (!closedEdges.Left())
      {
        radii.SetBottomLeft({});
        radii.SetTopLeft({});
      }
    }
  }

  BorderShape BorderShape::ShapeForBorderRect(const RenderStyle &style, const LayoutRect &borderRect,
                                              RectEdges<bool> closedEdges) noexcept
  {
    auto borderWidths = RectEdges<LayoutUnit> {
      style::Evaluate<LayoutUnit>(style.BorderTopWidth(), style::ZoomNeeded {}),
      style::Evaluate<LayoutUnit>(style.BorderRightWidth(), style::ZoomNeeded {}),
      style::Evaluate<LayoutUnit>(style.BorderBottomWidth(), style::ZoomNeeded {}),
      style::Evaluate<LayoutUnit>(style.BorderLeftWidth(), style::ZoomNeeded {}),
    };
    return ShapeForBorderRect(style, borderRect, borderWidths, closedEdges);
  }

  BorderShape BorderShape::ShapeForBorderRect(const RenderStyle &style, const LayoutRect &borderRect,
                                              const RectEdges<LayoutUnit> &overrideBorderWidths,
                                              RectEdges<bool> closedEdges) noexcept
  {
    // top, right, bottom, left.
    auto usedBorderWidths = RectEdges<LayoutUnit> {
      LayoutUnit(closedEdges.Top() ? overrideBorderWidths.Top() : 0_lu),
      LayoutUnit(closedEdges.Right() ? overrideBorderWidths.Right() : 0_lu),
      LayoutUnit(closedEdges.Bottom() ? overrideBorderWidths.Bottom() : 0_lu),
      LayoutUnit(closedEdges.Left() ? overrideBorderWidths.Left() : 0_lu),
    };

    if (style.HasBorderRadius())
    {
      auto radii =
        style::Evaluate<LayoutRoundedRectRadii>(style.BorderRadii(), borderRect.Size(), style::ZoomNeeded {});
      radii.Scale(CalcBorderRadiiConstraintScaleFor(borderRect, radii));

      ClearRadiiOfOpenEdges(radii, closedEdges);

      if (!radii.AreRenderableInRect(borderRect))
      {
        radii.MakeRenderableInRect(borderRect);
      }

      return BorderShape {borderRect, usedBorderWidths, radii};
    }

    return BorderShape {borderRect, usedBorderWidths};
  }

  BorderShape BorderShape::ShapeForOutsetRect(const RenderStyle &style, const LayoutRect &borderRect,
                                              const LayoutRect &outlineBoxRect,
                                              const RectEdges<LayoutUnit> &outlineWidths,
                                              RectEdges<bool> closedEdges) noexcept
  {
    // top, right, bottom, left.
    auto usedOutlineWidths = RectEdges<LayoutUnit> {
      LayoutUnit(closedEdges.Top() ? outlineWidths.Top() : 0_lu),
      LayoutUnit(closedEdges.Right() ? outlineWidths.Right() : 0_lu),
      LayoutUnit(closedEdges.Bottom() ? outlineWidths.Bottom() : 0_lu),
      LayoutUnit(closedEdges.Left() ? outlineWidths.Left() : 0_lu),
    };

    if (style.HasBorderRadius())
    {
      auto radii =
        style::Evaluate<LayoutRoundedRectRadii>(style.BorderRadii(), borderRect.Size(), style::ZoomNeeded {});

      auto leftOutset = std::max(borderRect.X() - outlineBoxRect.X(), 0_lu);
      auto topOutset = std::max(borderRect.Y() - outlineBoxRect.Y(), 0_lu);
      auto rightOutset = std::max(outlineBoxRect.MaxX() - borderRect.MaxX(), 0_lu);
      auto bottomOutset = std::max(outlineBoxRect.MaxY() - borderRect.MaxY(), 0_lu);

      radii.Expand(topOutset, bottomOutset, leftOutset, rightOutset);

      ClearRadiiOfOpenEdges(radii, closedEdges);

      if (!radii.AreRenderableInRect(outlineBoxRect))
      {
        radii.MakeRenderableInRect(outlineBoxRect);
      }

      return BorderShape {outlineBoxRect, usedOutlineWidths, radii};
    }

    return BorderShape {outlineBoxRect, usedOutlineWidths};
  }

  BorderShape BorderShape::ShapeForInsetRect(const RenderStyle &style, const LayoutRect &borderRect,
                                             const LayoutRect &insetRect) noexcept
  {
    if (style.HasBorderRadius())
    {
      auto radii =
        style::Evaluate<LayoutRoundedRectRadii>(style.BorderRadii(), borderRect.Size(), style::ZoomNeeded {});

      auto leftInset = std::max(insetRect.X() - borderRect.X(), 0_lu);
      auto topInset = std::max(insetRect.Y() - borderRect.Y(), 0_lu);
      auto rightInset = std::max(borderRect.MaxX() - insetRect.MaxX(), 0_lu);
      auto bottomInset = std::max(borderRect.MaxY() - insetRect.MaxY(), 0_lu);

      auto insetWidths = RectEdges<LayoutUnit> {topInset, rightInset, bottomInset, leftInset};
      auto roundedRect = LayoutRoundedRect {borderRect, radii};

      auto insetRoundedRect = ComputeInnerEdgeRoundedRect(roundedRect, insetWidths);

      return BorderShape {insetRect, RectEdges<LayoutUnit> {}, insetRoundedRect.GetRadii()};
    }

    return BorderShape {insetRect, RectEdges<LayoutUnit> {}};
  }

  BorderShape::BorderShape(const LayoutRect &borderRect, const RectEdges<LayoutUnit> &borderWidths) noexcept
      : _borderRect(borderRect), _innerEdgeRect(ComputeInnerEdgeRoundedRect(_borderRect, borderWidths)),
        _borderWidths(borderWidths)
  {
  }

  BorderShape::BorderShape(const LayoutRect &borderRect, const RectEdges<LayoutUnit> &borderWidths,
                           const LayoutRoundedRectRadii &radii) noexcept
      : _borderRect(borderRect, radii),
        _innerEdgeRect(ComputeInnerEdgeRoundedRect(_borderRect, borderWidths)), _borderWidths(borderWidths)
  {
    // The caller should have adjusted the radii already.
    krys_debug_assert(_borderRect.IsRenderable());
  }

  BorderShape BorderShape::ShapeWithBorderWidths(const RectEdges<LayoutUnit> &borderWidths) const noexcept
  {
    return BorderShape(_borderRect.Rect(), borderWidths, _borderRect.GetRadii());
  }

  LayoutRoundedRect BorderShape::DeprecatedRoundedRect() const noexcept
  {
    return _borderRect;
  }

  LayoutRoundedRect BorderShape::DeprecatedInnerRoundedRect() const noexcept
  {
    return _innerEdgeRect;
  }

  FloatRoundedRect BorderShape::DeprecatedPixelSnappedRoundedRect(float deviceScaleFactor) const noexcept
  {
    return _borderRect.PixelSnappedRoundedRectForPainting(deviceScaleFactor);
  }

  FloatRoundedRect BorderShape::DeprecatedPixelSnappedInnerRoundedRect(float deviceScaleFactor) const noexcept
  {
    return _innerEdgeRect.PixelSnappedRoundedRectForPainting(deviceScaleFactor);
  }

  FloatRect BorderShape::SnappedOuterRect(float deviceScaleFactor) const noexcept
  {
    return SnapRectToDevicePixels(_borderRect.Rect(), deviceScaleFactor);
  }

  FloatRect BorderShape::SnappedInnerRect(float deviceScaleFactor) const noexcept
  {
    return SnapRectToDevicePixels(InnerEdgeRect(), deviceScaleFactor);
  }

  bool BorderShape::InnerShapeContains(const LayoutRect &rect) const noexcept
  {
    return _innerEdgeRect.Contains(rect);
  }

  bool BorderShape::OuterShapeContains(const LayoutRect &rect) const noexcept
  {
    return _borderRect.Contains(rect);
  }

  bool BorderShape::OuterShapeIsRectangular() const noexcept
  {
    return !_borderRect.IsRounded();
  }

  bool BorderShape::InnerShapeIsRectangular() const noexcept
  {
    return !_innerEdgeRect.IsRounded();
  }

  void BorderShape::Move(LayoutSize offset) noexcept
  {
    _borderRect.Move(offset);
    _innerEdgeRect.Move(offset);
  }

  void BorderShape::Inflate(LayoutUnit amount) noexcept
  {
    _borderRect.InflateWithRadii(amount);
    _innerEdgeRect = ComputeInnerEdgeRoundedRect(_borderRect, _borderWidths);
  }

  LayoutRoundedRect
    BorderShape::ComputeInnerEdgeRoundedRect(const LayoutRoundedRect &borderRoundedRect,
                                             const RectEdges<LayoutUnit> &borderWidths) noexcept
  {
    auto borderRect = borderRoundedRect.Rect();
    auto width = std::max(0_lu, borderRect.Width() - borderWidths.Left() - borderWidths.Right());
    auto height = std::max(0_lu, borderRect.Height() - borderWidths.Top() - borderWidths.Bottom());
    auto innerRect =
      LayoutRect {borderRect.X() + borderWidths.Left(), borderRect.Y() + borderWidths.Top(), width, height};

    auto innerEdgeRect = LayoutRoundedRect {innerRect};
    if (borderRoundedRect.IsRounded())
    {
      auto innerRadii = borderRoundedRect.GetRadii();
      innerRadii.Shrink(borderWidths.Top(), borderWidths.Bottom(), borderWidths.Left(), borderWidths.Right());
      innerEdgeRect.SetRadii(innerRadii);

      if (!innerEdgeRect.IsRenderable())
      {
        innerEdgeRect.AdjustRadii();
      }
    }

    return innerEdgeRect;
  }
}
