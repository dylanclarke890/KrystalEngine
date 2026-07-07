#include "Krystal.HTML/SVG/SVGEllipseElement.hpp"

namespace Krys::HTML
{
  SVGEllipseElement::SVGEllipseElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Ellipse)
  {
  }
}