#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"

namespace krys::boo::svg
{
  SVGGraphicsElement::SVGGraphicsElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Graphics, SVGElementFlags::IsSvgGraphicsElement)
  {
  }
}