#include "Krystal.Booey/SVG/SVGMetadataElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGMetadataElement::SVGMetadataElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Metadata)
  {
  }
}