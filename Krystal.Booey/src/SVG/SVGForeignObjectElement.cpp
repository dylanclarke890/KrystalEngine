#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"

namespace krys::boo::svg
{
  SVGForeignObjectElement::SVGForeignObjectElement(Document &document) noexcept
      : SVGElement(document, SVGElementInterface::ForeignObject)
  {
  }
}