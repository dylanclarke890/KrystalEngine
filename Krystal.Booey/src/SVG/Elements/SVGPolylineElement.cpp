#include "Krystal.Booey/SVG/Elements/SVGPolylineElement.hpp"

namespace krys::boo::svg
{
  SVGPolylineElement::SVGPolylineElement(dom::Document &document) noexcept
      : SVGPolyElement(document, SVGElementInterface::Polyline)
  {
  }
}
