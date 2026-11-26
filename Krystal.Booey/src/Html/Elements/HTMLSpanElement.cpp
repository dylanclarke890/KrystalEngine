#include "Krystal.Booey/Html/Elements/HTMLSpanElement.hpp"

namespace krys::boo
{
  HTMLSpanElement::HTMLSpanElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Span)
  {
  }
}
