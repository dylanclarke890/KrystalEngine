#include "Krystal.HTML/SVG/SVGSymbolElement.hpp"

namespace Krys::HTML
{
  SVGSymbolElement::SVGSymbolElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Symbol)
  {
  }
}