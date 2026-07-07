#include "Krystal.HTML/SVG/SVGForeignObjectElement.hpp"

namespace Krys::HTML
{
  SVGForeignObjectElement::SVGForeignObjectElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::ForeignObject)
  {
  }
}