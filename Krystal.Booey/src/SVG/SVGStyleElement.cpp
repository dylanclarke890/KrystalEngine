#include "Krystal.Booey/SVG/SVGStyleElement.hpp"

namespace krys::boo::svg
{
  SVGStyleElement::SVGStyleElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Style)
  {
  }
}