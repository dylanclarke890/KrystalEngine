#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::html
{
  HTMLUnknownElement::HTMLUnknownElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Unknown, HTMLElementFlags::None,
                    dom::NodeFlags::IsUnknownElement)
  {
  }
}