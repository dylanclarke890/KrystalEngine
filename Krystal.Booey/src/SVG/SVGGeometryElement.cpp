#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"

namespace krys::boo::svg
{
  SVGGeometryElement::SVGGeometryElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Geometry, SVGElementFlags::IsSvgGeometryElement)
  {
  }
}