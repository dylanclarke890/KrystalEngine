#include "Krystal.Booey/SVG/SVGPolylineElement.hpp"

namespace krys::boo::svg
{
  SVGPolylineElement::SVGPolylineElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Polyline)
  {
  }
}