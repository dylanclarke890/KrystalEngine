#include "Krystal.Booey/SVG/SVGPolylineElement.hpp"

namespace krys::boo::svg
{
  SVGPolylineElement::SVGPolylineElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Polyline)
  {
  }
}