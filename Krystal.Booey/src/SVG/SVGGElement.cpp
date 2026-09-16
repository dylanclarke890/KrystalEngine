#include "Krystal.Booey/SVG/SVGGElement.hpp"

namespace krys::boo::svg
{
  SVGGElement::SVGGElement(dom::Document &document) noexcept : SVGElement(document, SVGElementInterface::G)
  {
  }
}