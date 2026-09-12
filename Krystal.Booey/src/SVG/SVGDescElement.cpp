#include "Krystal.Booey/SVG/SVGDescElement.hpp"

namespace krys::boo::svg
{
  SVGDescElement::SVGDescElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Desc)
  {
  }
}