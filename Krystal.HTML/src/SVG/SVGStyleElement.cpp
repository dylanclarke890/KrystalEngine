#include "Krystal.HTML/SVG/SVGStyleElement.hpp"

namespace Krys::HTML
{
  SVGStyleElement::SVGStyleElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Style)
  {
  }
}