#pragma once

#include "Krystal.Booey/SVG/SVGPolyElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<polygon>` element: a closed shape through a list of points.
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGPolygonElement
  class SVGPolygonElement final : public SVGPolyElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPolygonElement);

  public:
    explicit SVGPolygonElement(dom::Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGPolygonElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGPolygonElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
