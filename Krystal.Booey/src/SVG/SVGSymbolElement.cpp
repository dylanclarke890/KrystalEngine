#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"

namespace krys::boo::svg
{
  SVGSymbolElement::SVGSymbolElement(dom::Document &document) noexcept
      : SVGElement(document, SVGElementInterface::Symbol)
  {
  }
}