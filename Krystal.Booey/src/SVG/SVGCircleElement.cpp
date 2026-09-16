#include "Krystal.Booey/SVG/SVGCircleElement.hpp"

namespace krys::boo::svg
{
  SVGCircleElement::SVGCircleElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Circle)
  {
  }
}