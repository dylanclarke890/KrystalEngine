#include "Krystal.Booey/SVG/Elements/SVGDescElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo
{
  SVGDescElement::SVGDescElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Desc)
  {
  }
}
