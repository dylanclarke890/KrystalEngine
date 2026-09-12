#include "Krystal.Booey/SVG/SVGScriptElement.hpp"

namespace krys::boo::svg
{
  SVGScriptElement::SVGScriptElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Script)
  {
  }
}