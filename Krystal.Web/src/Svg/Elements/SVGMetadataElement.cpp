#include "Krystal.Web/Svg/Elements/SVGMetadataElement.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::web
{
  SVGMetadataElement::SVGMetadataElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Metadata)
  {
  }
}
