#include "Krystal.Booey/SVG/SVGRectElement.hpp"

namespace krys::boo::svg
{
  SVGRectElement::SVGRectElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Rect)
  {
  }
}