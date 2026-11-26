#include "Krystal.Web/Svg/Elements/SVGScriptElement.hpp"

namespace krys::web
{
  SVGScriptElement::SVGScriptElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Script)
  {
  }
}
