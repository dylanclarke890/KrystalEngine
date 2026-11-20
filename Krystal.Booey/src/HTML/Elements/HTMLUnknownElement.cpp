#include "Krystal.Booey/HTML/Elements/HTMLUnknownElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"

namespace krys::boo
{
  HTMLUnknownElement::HTMLUnknownElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Unknown, html::HTMLElementFlags::None,
                    dom::NodeFlags::IsUnknownElement)
  {
  }
}
