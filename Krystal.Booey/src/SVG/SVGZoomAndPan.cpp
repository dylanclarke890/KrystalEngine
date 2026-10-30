#include "Krystal.Booey/SVG/SVGZoomAndPan.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Text/ParsingUtilities.hpp"

namespace krys::boo::svg
{
  Maybe<SVGZoomAndPanType> SVGZoomAndPan::ParseZoomAndPan(dom::DOMStringView &buffer) noexcept
  {
    if (text::SkipCharactersExactly(buffer, u8"disable"))
    {
      return SVGZoomAndPanDisable;
    }

    if (text::SkipCharactersExactly(buffer, u8"magnify"))
    {
      return SVGZoomAndPanMagnify;
    }

    return {};
  }

  void SVGZoomAndPan::ParseAttribute(SVGElement &element, const dom::QualifiedName &attributeName,
                                     Maybe<dom::DOMStringView> value) noexcept
  {
    if (attributeName != SVGNames::ZoomAndPanAttr())
    {
      return;
    }

    // NOTE: An absent attribute is magnify, its initial value.
    if (!value)
    {
      Reset();
      return;
    }
    _zoomAndPan = SVGPropertyTraits<SVGZoomAndPanType>::FromString(element, *value);
  }
}
