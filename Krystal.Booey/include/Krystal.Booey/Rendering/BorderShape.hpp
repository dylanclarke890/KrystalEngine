#pragma once

#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class Color;
  class FloatRect;
  class FloatRoundedRect;
  class GraphicsContext;
  class Path;
  class RenderStyle;

  /// @brief The shape formed by the border and padding boxes with border-radius, used to fill and clip.
  /// @note In future, this may be a more complex shape than a rounded rect, so accessors that return rounded
  /// rects are deprecated.
  class BorderShape
  {
  private:
    LayoutRoundedRect _borderRect;
    LayoutRoundedRect _innerEdgeRect;
    RectEdges<LayoutUnit> _borderWidths;

  public:
    KRYS_NODISCARD static BorderShape ShapeForBorderRect(const RenderStyle &style,
                                                         const LayoutRect &borderRect,
                                                         RectEdges<bool> closedEdges = {true}) noexcept;
    /// @param overrideBorderWidths Custom insets from the border box, used instead of the border widths from
    /// the style.
    KRYS_NODISCARD static BorderShape ShapeForBorderRect(const RenderStyle &style,
                                                         const LayoutRect &borderRect,
                                                         const RectEdges<LayoutUnit> &overrideBorderWidths,
                                                         RectEdges<bool> closedEdges = {true}) noexcept;

    /// @brief A shape suitable for rendering an outline or outset shadow. @p borderRect is provided to allow
    /// for scaling the corner radii.
    KRYS_NODISCARD static BorderShape ShapeForOutsetRect(const RenderStyle &style,
                                                         const LayoutRect &borderRect,
                                                         const LayoutRect &outlineBoxRect,
                                                         const RectEdges<LayoutUnit> &outlineWidths,
                                                         RectEdges<bool> closedEdges = {true}) noexcept;

    /// @brief A shape suitable for rendering a shape inset from the box. @p borderRect is provided to allow
    /// for scaling the corner radii.
    KRYS_NODISCARD static BorderShape ShapeForInsetRect(const RenderStyle &style,
                                                        const LayoutRect &borderRect,
                                                        const LayoutRect &insetRect) noexcept;

    BorderShape(const LayoutRect &borderRect, const RectEdges<LayoutUnit> &borderWidths) noexcept;
    BorderShape(const LayoutRect &borderRect, const RectEdges<LayoutUnit> &borderWidths,
                const LayoutRoundedRectRadii &radii) noexcept;

    KRYS_NODISCARD BorderShape
      ShapeWithBorderWidths(const RectEdges<LayoutUnit> &borderWidths) const noexcept;

    KRYS_NODISCARD LayoutRect BorderRect() const noexcept
    {
      return _borderRect.Rect();
    }
    KRYS_NODISCARD LayoutRect InnerEdgeRect() const noexcept
    {
      return _innerEdgeRect.Rect();
    }

    /// @brief The border widths, taking the closed edges into account.
    KRYS_NODISCARD const RectEdges<LayoutUnit> &BorderWidths() const noexcept
    {
      return _borderWidths;
    }

    KRYS_NODISCARD LayoutRoundedRect DeprecatedRoundedRect() const noexcept;
    KRYS_NODISCARD LayoutRoundedRect DeprecatedInnerRoundedRect() const noexcept;
    KRYS_NODISCARD FloatRoundedRect DeprecatedPixelSnappedRoundedRect(float deviceScaleFactor) const noexcept;
    KRYS_NODISCARD FloatRoundedRect
      DeprecatedPixelSnappedInnerRoundedRect(float deviceScaleFactor) const noexcept;

    /// @brief Whether @p rect is entirely inside the shape, without impinging on any of the corners.
    KRYS_NODISCARD bool InnerShapeContains(const LayoutRect &rect) const noexcept;
    KRYS_NODISCARD bool OuterShapeContains(const LayoutRect &rect) const noexcept;

    KRYS_NODISCARD const LayoutRoundedRectRadii &Radii() const noexcept
    {
      return _borderRect.GetRadii();
    }
    void SetRadii(const LayoutRoundedRectRadii &radii) noexcept
    {
      _borderRect.SetRadii(radii);
    }

    /// @brief Where the straight edge sections of the inner edge terminate. The inner edge isn't necessarily
    /// a rounded rect.
    KRYS_NODISCARD const LayoutRoundedRectRadii &InnerEdgeRadii() const noexcept
    {
      return _innerEdgeRect.GetRadii();
    }

    KRYS_NODISCARD FloatRect SnappedOuterRect(float deviceScaleFactor) const noexcept;
    KRYS_NODISCARD FloatRect SnappedInnerRect(float deviceScaleFactor) const noexcept;

    KRYS_NODISCARD bool IsRounded() const noexcept
    {
      return _borderRect.IsRounded();
    }

    KRYS_NODISCARD bool OuterShapeIsRectangular() const noexcept;
    KRYS_NODISCARD bool InnerShapeIsRectangular() const noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _borderRect.Rect().IsEmpty();
    }

    void Move(LayoutSize offset) noexcept;

    /// @brief Inflates the border rect and scales the radii up accordingly. Note that this changes the
    /// meaning of the "inner shape", which no longer corresponds to the padding box.
    void Inflate(LayoutUnit amount) noexcept;

    KRYS_NODISCARD Path PathForOuterShape(float deviceScaleFactor) const noexcept;
    KRYS_NODISCARD Path PathForInnerShape(float deviceScaleFactor) const noexcept;

    void AddOuterShapeToPath(Path &, float deviceScaleFactor) const noexcept;
    void AddInnerShapeToPath(Path &, float deviceScaleFactor) const noexcept;

    KRYS_NODISCARD Path PathForBorderArea(float deviceScaleFactor) const noexcept;

    void ClipToOuterShape(GraphicsContext &, float deviceScaleFactor) const noexcept;
    void ClipToInnerShape(GraphicsContext &, float deviceScaleFactor) const noexcept;

    void ClipOutOuterShape(GraphicsContext &, float deviceScaleFactor) const noexcept;
    void ClipOutInnerShape(GraphicsContext &, float deviceScaleFactor) const noexcept;

    void FillOuterShape(GraphicsContext &, const Color &, float deviceScaleFactor) const noexcept;
    void FillInnerShape(GraphicsContext &, const Color &, float deviceScaleFactor) const noexcept;

    void FillRectWithInnerHoleShape(GraphicsContext &, const LayoutRect &outerRect, const Color &,
                                    float deviceScaleFactor) const noexcept;

  private:
    KRYS_NODISCARD static LayoutRoundedRect
      ComputeInnerEdgeRoundedRect(const LayoutRoundedRect &borderRoundedRect,
                                  const RectEdges<LayoutUnit> &borderWidths) noexcept;
  };
}
