#include "Krystal.HTML/SVG/SVGUnknownElement.hpp"

namespace Krys::HTML
{
  SVGUnknownElement::SVGUnknownElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Unknown)
  {
  }
}