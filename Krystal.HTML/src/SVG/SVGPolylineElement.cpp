#include "Krystal.HTML/SVG/SVGPolylineElement.hpp"

namespace Krys::HTML
{
  SVGPolylineElement::SVGPolylineElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Polyline)
  {
  }
}