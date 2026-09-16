#include "Krystal.Booey/SVG/SVGSvgElement.hpp"

namespace krys::boo::svg
{
  SVGSvgElement::SVGSvgElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Svg)
  {
  }
}