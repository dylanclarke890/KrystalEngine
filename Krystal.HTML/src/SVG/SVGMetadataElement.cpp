#include "Krystal.HTML/SVG/SVGMetadataElement.hpp"

namespace Krys::HTML
{
  SVGMetadataElement::SVGMetadataElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Metadata)
  {
  }
}