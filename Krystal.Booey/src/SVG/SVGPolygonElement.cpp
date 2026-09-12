#include "Krystal.Booey/SVG/SVGPolygonElement.hpp"

namespace krys::boo::svg
{
  SVGPolygonElement::SVGPolygonElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Polygon)
  {
  }
}