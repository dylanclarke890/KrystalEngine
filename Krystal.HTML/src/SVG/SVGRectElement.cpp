#include "Krystal.HTML/SVG/SVGRectElement.hpp"

namespace Krys::HTML
{
  SVGRectElement::SVGRectElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Rect)
  {
  }
}