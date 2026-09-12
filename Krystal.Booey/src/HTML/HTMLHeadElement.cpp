#include "Krystal.Booey/HTML/HTMLHeadElement.hpp"

namespace krys::boo::html
{
  HTMLHeadElement::HTMLHeadElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Head)
  {
  }
}
