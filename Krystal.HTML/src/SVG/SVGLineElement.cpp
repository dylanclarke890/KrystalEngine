#include "Krystal.HTML/SVG/SVGLineElement.hpp"

namespace Krys::HTML
{
  SVGLineElement::SVGLineElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Line)
  {
  }
}