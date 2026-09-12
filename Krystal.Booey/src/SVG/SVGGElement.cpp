#include "Krystal.Booey/SVG/SVGGElement.hpp"

namespace krys::boo::svg
{
  SVGGElement::SVGGElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::G)
  {
  }
}