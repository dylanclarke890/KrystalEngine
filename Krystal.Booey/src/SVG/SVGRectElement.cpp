#include "Krystal.Booey/SVG/SVGRectElement.hpp"

namespace krys::boo::svg
{
  SVGRectElement::SVGRectElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Rect)
  {
  }
}