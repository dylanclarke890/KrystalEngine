#include "Krystal.Booey/SVG/SVGMetadataElement.hpp"

namespace krys::boo::svg
{
  SVGMetadataElement::SVGMetadataElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Metadata)
  {
  }
}