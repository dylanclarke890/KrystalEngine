#include "Krystal.Booey/SVG/SVGTitleElement.hpp"

namespace krys::boo::svg
{
  SVGTitleElement::SVGTitleElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Title)
  {
  }
}