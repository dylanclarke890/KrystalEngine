#include "Krystal.Booey/SVG/SVGImageElement.hpp"

namespace krys::boo::svg
{
  SVGImageElement::SVGImageElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Image)
  {
  }
}