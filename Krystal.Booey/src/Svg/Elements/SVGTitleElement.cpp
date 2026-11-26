#include "Krystal.Booey/Svg/Elements/SVGTitleElement.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo
{
  SVGTitleElement::SVGTitleElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Title)
  {
  }
}
