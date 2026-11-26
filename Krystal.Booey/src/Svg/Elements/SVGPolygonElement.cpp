#include "Krystal.Booey/Svg/Elements/SVGPolygonElement.hpp"

namespace krys::boo
{
  SVGPolygonElement::SVGPolygonElement(Document &document) noexcept
      : SVGPolyElement(document, SVGElementInterface::Polygon)
  {
  }
}
