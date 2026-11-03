#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"

namespace krys::boo::html
{
  HTMLHeadElement::HTMLHeadElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Head)
  {
  }
}
