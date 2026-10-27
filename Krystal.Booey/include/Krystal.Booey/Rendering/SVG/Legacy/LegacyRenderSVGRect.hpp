#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"

namespace krys::boo::svg
{
  class SVGRectElement;
}

namespace krys::boo
{
  /// @brief The renderer of `<rect>`: plain rectangles are filled, stroked and hit tested without a path.
  class LegacyRenderSVGRect final : public LegacyRenderSVGShape
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGRect);

  public:
    LegacyRenderSVGRect(svg::SVGRectElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGRect() noexcept override;

    KRYS_NODISCARD svg::SVGRectElement &RectElement() const noexcept;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGRect";
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

    KRYS_NODISCARD bool DefinitelyHasSimpleStroke() const noexcept;
    KRYS_NODISCARD bool CanUseStrokeHitTestFastPath() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGRect)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGRect();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
