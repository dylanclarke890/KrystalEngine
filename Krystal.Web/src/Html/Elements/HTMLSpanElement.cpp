#include "Krystal.Web/Html/Elements/HTMLSpanElement.hpp"

namespace krys::web
{
  HTMLSpanElement::HTMLSpanElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Span)
  {
  }
}
