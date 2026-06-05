#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"

namespace Krys::HTML
{
  HTMLSlotElement::HTMLSlotElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Slot, HTMLElementFlags::None, NodeFlags::IsHTMLSlotElement)
  {
  }
}