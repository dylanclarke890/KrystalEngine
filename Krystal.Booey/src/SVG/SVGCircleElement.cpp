#include "Krystal.Booey/SVG/SVGCircleElement.hpp"

namespace krys::boo::svg
{
  SVGCircleElement::SVGCircleElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Circle)
  {
  }
}