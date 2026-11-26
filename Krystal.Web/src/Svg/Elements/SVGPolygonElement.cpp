#include "Krystal.Web/Svg/Elements/SVGPolygonElement.hpp"

namespace krys::web
{
  SVGPolygonElement::SVGPolygonElement(Document &document) noexcept
      : SVGPolyElement(document, SVGElementInterface::Polygon)
  {
  }
}
