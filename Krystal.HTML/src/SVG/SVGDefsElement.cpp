#include "Krystal.HTML/SVG/SVGDefsElement.hpp"

namespace Krys::HTML
{
  SVGDefsElement::SVGDefsElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Defs)
  {
  }
}