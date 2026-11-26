#include "Krystal.Booey/Svg/Elements/SVGMetadataElement.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo
{
  SVGMetadataElement::SVGMetadataElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Metadata)
  {
  }
}
