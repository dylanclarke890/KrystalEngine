#pragma once

#include "Krystal.Web/Svg/Elements/SVGPolyElement.hpp"

namespace krys::web
{
  /// @brief The `<polyline>` element: an open shape through a list of points.
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGPolylineElement
  class SVGPolylineElement final : public SVGPolyElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPolylineElement);

  public:
    explicit SVGPolylineElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGPolylineElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGPolylineElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
