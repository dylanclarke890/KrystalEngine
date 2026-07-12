#pragma once

#include "Krystal.HTML/SVG/SVGElement.hpp"

namespace Krys::HTML
{
  class SVGGraphicsElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGGraphicsElement);

  public:
    SVGGraphicsElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::SVGGraphicsElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::SVGElement &target) noexcept
  {
    return target.IsSVGGraphicsElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();