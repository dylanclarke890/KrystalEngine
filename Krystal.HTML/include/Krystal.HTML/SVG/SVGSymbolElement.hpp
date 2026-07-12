#pragma once

#include "Krystal.HTML/SVG/SVGElement.hpp"

namespace Krys::HTML
{
  class SVGSymbolElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSymbolElement);

  public:
    SVGSymbolElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::SVGSymbolElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::SVGElement &target) noexcept
  {
    return target.IsSVGSymbolElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();