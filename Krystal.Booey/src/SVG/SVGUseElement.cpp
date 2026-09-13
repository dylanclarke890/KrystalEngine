#include "Krystal.Booey/SVG/SVGUseElement.hpp"

namespace krys::boo::svg
{
  SVGUseElement::SVGUseElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Use)
  {
  }
}