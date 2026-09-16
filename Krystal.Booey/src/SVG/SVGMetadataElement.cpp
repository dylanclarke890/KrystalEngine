#include "Krystal.Booey/SVG/SVGMetadataElement.hpp"

namespace krys::boo::svg
{
  SVGMetadataElement::SVGMetadataElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Metadata)
  {
  }
}