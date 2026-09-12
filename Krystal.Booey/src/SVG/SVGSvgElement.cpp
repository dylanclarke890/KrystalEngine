#include "Krystal.Booey/SVG/SVGSvgElement.hpp"

namespace krys::boo::svg
{
  SVGSvgElement::SVGSvgElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Svg)
  {
  }
}