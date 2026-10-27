#include "Krystal.Booey/SVG/SVGDescElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGDescElement::SVGDescElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Desc)
  {
  }
}