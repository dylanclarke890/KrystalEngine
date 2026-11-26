#pragma once

#include "Krystal.Web/Svg/Elements/SVGPolyElement.hpp"

namespace krys::web
{
  /// @brief The `<polygon>` element: a closed shape through a list of points.
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGPolygonElement
  class SVGPolygonElement final : public SVGPolyElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPolygonElement);

  public:
    explicit SVGPolygonElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGPolygonElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGPolygonElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
