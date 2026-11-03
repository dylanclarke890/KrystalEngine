#include "Krystal.Booey/SVG/Elements/SVGScriptElement.hpp"

namespace krys::boo::svg
{
  SVGScriptElement::SVGScriptElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Script)
  {
  }
}