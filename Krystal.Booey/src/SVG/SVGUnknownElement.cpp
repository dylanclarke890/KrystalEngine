#include "Krystal.Booey/SVG/SVGUnknownElement.hpp"

namespace krys::boo::svg
{
  SVGUnknownElement::SVGUnknownElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Unknown)
  {
  }
}