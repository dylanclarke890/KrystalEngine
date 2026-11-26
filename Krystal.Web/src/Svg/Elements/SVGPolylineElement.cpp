#include "Krystal.Web/Svg/Elements/SVGPolylineElement.hpp"

namespace krys::web
{
  SVGPolylineElement::SVGPolylineElement(Document &document) noexcept
      : SVGPolyElement(document, SVGElementInterface::Polyline)
  {
  }
}
