#include "Krystal.Booey/SVG/SVGUnknownElement.hpp"

namespace krys::boo::svg
{
  SVGUnknownElement::SVGUnknownElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Unknown)
  {
  }
}