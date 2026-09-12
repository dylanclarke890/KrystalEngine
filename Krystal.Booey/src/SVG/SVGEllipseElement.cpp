#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"

namespace krys::boo::svg
{
  SVGEllipseElement::SVGEllipseElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Ellipse)
  {
  }
}