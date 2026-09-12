#include "Krystal.Booey/SVG/SVGDefsElement.hpp"

namespace krys::boo::svg
{
  SVGDefsElement::SVGDefsElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Defs)
  {
  }
}