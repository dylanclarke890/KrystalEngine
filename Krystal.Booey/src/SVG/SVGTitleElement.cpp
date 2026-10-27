#include "Krystal.Booey/SVG/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGTitleElement::SVGTitleElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Title)
  {
  }
}