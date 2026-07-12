#include "Krystal.HTML/SVG/SVGPolygonElement.hpp"

namespace Krys::HTML
{
  SVGPolygonElement::SVGPolygonElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Polygon)
  {
  }
}