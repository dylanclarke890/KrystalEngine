#include "Krystal.HTML/SVG/SVGImageElement.hpp"

namespace Krys::HTML
{
  SVGImageElement::SVGImageElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Image)
  {
  }
}