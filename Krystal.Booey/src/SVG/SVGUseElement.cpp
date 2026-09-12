#include "Krystal.Booey/SVG/SVGUseElement.hpp"

namespace krys::boo::svg
{
  SVGUseElement::SVGUseElement(Document &document) noexcept : SVGElement(document, SVGElementInterface::Use)
  {
  }
}