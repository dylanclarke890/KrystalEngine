#include "Krystal.HTML/SVG/SVGTitleElement.hpp"

namespace Krys::HTML
{
  SVGTitleElement::SVGTitleElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Title)
  {
  }
}