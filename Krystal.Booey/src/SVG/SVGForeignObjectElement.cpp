#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"

namespace krys::boo::svg
{
  SVGForeignObjectElement::SVGForeignObjectElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::ForeignObject)
  {
  }
}