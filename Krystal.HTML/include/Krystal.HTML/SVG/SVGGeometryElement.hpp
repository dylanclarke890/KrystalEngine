#pragma once

#include "Krystal.HTML/SVG/SVGElement.hpp"

namespace Krys::HTML
{
  class SVGGeometryElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGGeometryElement);

  public:
    SVGGeometryElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::SVGGeometryElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::SVGElement &target) noexcept
  {
    return target.IsSVGGeometryElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();