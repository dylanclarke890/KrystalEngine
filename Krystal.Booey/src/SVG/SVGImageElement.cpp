#include "Krystal.Booey/SVG/SVGImageElement.hpp"

namespace krys::boo::svg
{
  SVGImageElement::SVGImageElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Image)
  {
  }
}