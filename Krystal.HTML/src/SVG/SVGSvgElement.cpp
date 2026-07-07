#include "Krystal.HTML/SVG/SVGSvgElement.hpp"

namespace Krys::HTML
{
  SVGSvgElement::SVGSvgElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Svg)
  {
  }
}