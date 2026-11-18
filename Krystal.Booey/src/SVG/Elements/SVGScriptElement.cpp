#include "Krystal.Booey/SVG/Elements/SVGScriptElement.hpp"

namespace krys::boo
{
  SVGScriptElement::SVGScriptElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Script)
  {
  }
}
