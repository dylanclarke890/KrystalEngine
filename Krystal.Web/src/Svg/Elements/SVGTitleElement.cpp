#include "Krystal.Web/Svg/Elements/SVGTitleElement.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::web
{
  SVGTitleElement::SVGTitleElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Title)
  {
  }
}
