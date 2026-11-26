#include "Krystal.Booey/Svg/Elements/SVGScriptElement.hpp"

namespace krys::boo
{
  SVGScriptElement::SVGScriptElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Script)
  {
  }
}
