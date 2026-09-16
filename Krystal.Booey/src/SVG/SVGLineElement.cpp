#include "Krystal.Booey/SVG/SVGLineElement.hpp"

namespace krys::boo::svg
{
  SVGLineElement::SVGLineElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Line)
  {
  }
}