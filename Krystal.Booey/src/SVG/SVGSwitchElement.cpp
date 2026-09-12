#include "Krystal.Booey/SVG/SVGSwitchElement.hpp"

namespace krys::boo::svg
{
  SVGSwitchElement::SVGSwitchElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Switch)
  {
  }
}