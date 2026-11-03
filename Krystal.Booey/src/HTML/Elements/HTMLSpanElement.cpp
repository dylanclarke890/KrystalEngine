#include "Krystal.Booey/HTML/Elements/HTMLSpanElement.hpp"

namespace krys::boo::html
{
  HTMLSpanElement::HTMLSpanElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Span)
  {
  }
}