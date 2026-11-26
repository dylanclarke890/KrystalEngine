#include "Krystal.Web/Html/Elements/HTMLUnknownElement.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web
{
  HTMLUnknownElement::HTMLUnknownElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Unknown, {}, dom::NodeFlag::IsUnknownElement)
  {
  }
}
