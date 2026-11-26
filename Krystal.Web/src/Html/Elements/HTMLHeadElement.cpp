#include "Krystal.Web/Html/Elements/HTMLHeadElement.hpp"

namespace krys::web
{
  HTMLHeadElement::HTMLHeadElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Head)
  {
  }
}
