#include "Krystal.HTML/SVG/SVGGeometryElement.hpp"

namespace Krys::HTML
{
  SVGGeometryElement::SVGGeometryElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Geometry, SVGElementFlags::IsSvgGeometryElement)
  {
  }
}