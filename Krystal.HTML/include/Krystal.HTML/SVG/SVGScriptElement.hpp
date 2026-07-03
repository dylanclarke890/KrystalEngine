#pragma once

#include "Krystal.HTML/SVG/SVGElement.hpp"

namespace Krys::HTML
{
  class SVGScriptElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGScriptElement);

  public:
    SVGScriptElement(Document &document) noexcept : SVGElement(document, SVGTagName::script)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::SVGScriptElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::SVGElement &target) noexcept
  {
    return target.IsSVGScriptElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();