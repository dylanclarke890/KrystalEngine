#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"

namespace krys::boo
{
  /// @brief The renderer of `<circle>` and `<ellipse>`: they're filled, stroked and hit tested without a
  /// path.
  class LegacyRenderSVGEllipse final : public LegacyRenderSVGShape
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGEllipse);

  private:
    FloatPoint _center;
    FloatSize _radii;

  public:
    LegacyRenderSVGEllipse(svg::SVGGraphicsElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGEllipse() noexcept override;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGEllipse";
    }

    void UpdateShapeFromElement() noexcept override;
    KRYS_NODISCARD bool IsEmpty() const noexcept override
    {
      return HasPath() ? LegacyRenderSVGShape::IsEmpty() : _fillBoundingBox.IsEmpty();
    }
    KRYS_NODISCARD bool IsRenderingDisabled() const noexcept override;
    void FillShape(GraphicsContext &) const noexcept override;
    void StrokeShape(GraphicsContext &) const noexcept override;
    KRYS_NODISCARD bool
      ShapeDependentStrokeContains(const FloatPoint &,
                                   PointCoordinateSpace = GlobalCoordinateSpace) noexcept override;
    KRYS_NODISCARD bool ShapeDependentFillContains(const FloatPoint &,
                                                   const WindRule) const noexcept override;
    void CalculateRadiiAndCenter() noexcept;

    KRYS_NODISCARD bool CanUseStrokeHitTestFastPath() const noexcept;
  };
}
