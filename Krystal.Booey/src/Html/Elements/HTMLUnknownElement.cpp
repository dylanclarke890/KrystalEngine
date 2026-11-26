#include "Krystal.Booey/Html/Elements/HTMLUnknownElement.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"

namespace krys::boo
{
  HTMLUnknownElement::HTMLUnknownElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Unknown, {}, dom::NodeFlag::IsUnknownElement)
  {
  }
}
