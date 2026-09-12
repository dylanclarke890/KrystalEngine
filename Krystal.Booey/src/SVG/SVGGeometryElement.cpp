#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"

namespace krys::boo::svg
{
  SVGGeometryElement::SVGGeometryElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Geometry, SVGElementFlags::IsSvgGeometryElement)
  {
  }
}