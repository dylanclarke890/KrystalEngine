#include "Krystal.Booey/Html/Elements/HTMLHeadElement.hpp"

namespace krys::boo
{
  HTMLHeadElement::HTMLHeadElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Head)
  {
  }
}
