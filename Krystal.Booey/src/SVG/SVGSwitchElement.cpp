#include "Krystal.Booey/SVG/SVGSwitchElement.hpp"

namespace krys::boo::svg
{
  SVGSwitchElement::SVGSwitchElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Switch)
  {
  }
}