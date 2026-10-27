#pragma once

#include "Krystal.Booey/SVG/SVGPolyElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<polyline>` element: an open shape through a list of points.
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGPolylineElement
  class SVGPolylineElement final : public SVGPolyElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPolylineElement);

  public:
    explicit SVGPolylineElement(dom::Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGPolylineElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGPolylineElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
