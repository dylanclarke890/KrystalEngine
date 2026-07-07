#include "Krystal.HTML/SVG/SVGScriptElement.hpp"

namespace Krys::HTML
{
  SVGScriptElement::SVGScriptElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Script)
  {
  }
}