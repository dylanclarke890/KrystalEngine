#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"

namespace krys::boo::svg
{
  SVGEllipseElement::SVGEllipseElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Ellipse)
  {
  }
}