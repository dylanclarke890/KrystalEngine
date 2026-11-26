#include "Krystal.Web/Svg/Elements/SVGDescElement.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::web
{
  SVGDescElement::SVGDescElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Desc)
  {
  }
}
