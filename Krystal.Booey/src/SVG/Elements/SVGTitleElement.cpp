#include "Krystal.Booey/SVG/Elements/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo
{
  SVGTitleElement::SVGTitleElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Title)
  {
  }
}
