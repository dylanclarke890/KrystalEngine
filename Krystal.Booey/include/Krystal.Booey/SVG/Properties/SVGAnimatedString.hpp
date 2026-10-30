#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPrimitiveProperty.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGAnimatedString
  class SVGAnimatedString : public SVGAnimatedPrimitiveProperty<dom::DOMString>
  {
  public:
    KRYS_NODISCARD static Ref<SVGAnimatedString> Create(SVGElement *contextElement) noexcept
    {
      return AdoptRef(*new SVGAnimatedString(contextElement));
    }

  protected:
    SVGAnimatedString(SVGElement *contextElement) noexcept
        : SVGAnimatedPrimitiveProperty<dom::DOMString>(contextElement)
    {
    }
  };
}
