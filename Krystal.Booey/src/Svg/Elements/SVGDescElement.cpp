#include "Krystal.Booey/Svg/Elements/SVGDescElement.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo
{
  SVGDescElement::SVGDescElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Desc)
  {
  }
}
