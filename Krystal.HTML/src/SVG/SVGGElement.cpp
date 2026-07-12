#include "Krystal.HTML/SVG/SVGGElement.hpp"

namespace Krys::HTML
{
  SVGGElement::SVGGElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::G)
  {
  }
}