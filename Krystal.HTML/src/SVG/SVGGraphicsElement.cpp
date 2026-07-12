#include "Krystal.HTML/SVG/SVGGraphicsElement.hpp"

namespace Krys::HTML
{
  SVGGraphicsElement::SVGGraphicsElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Graphics, SVGElementFlags::IsSvgGraphicsElement)
  {
  }
}