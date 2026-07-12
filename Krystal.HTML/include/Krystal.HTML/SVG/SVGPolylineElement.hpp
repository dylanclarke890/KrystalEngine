#pragma once

#include "Krystal.HTML/SVG/SVGElement.hpp"

namespace Krys::HTML
{
  class SVGPolylineElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPolylineElement);

  public:
    SVGPolylineElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::SVGPolylineElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::SVGElement &target) noexcept
  {
    return target.IsSVGPolylineElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();