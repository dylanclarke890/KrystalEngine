#include "Krystal.HTML/SVG/SVGDescElement.hpp"

namespace Krys::HTML
{
  SVGDescElement::SVGDescElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Desc)
  {
  }
}