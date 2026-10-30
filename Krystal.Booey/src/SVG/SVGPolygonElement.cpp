#include "Krystal.Booey/SVG/SVGPolygonElement.hpp"

namespace krys::boo::svg
{
  SVGPolygonElement::SVGPolygonElement(dom::Document &document) noexcept
      : SVGPolyElement(document, SVGElementInterface::Polygon)
  {
  }
}
