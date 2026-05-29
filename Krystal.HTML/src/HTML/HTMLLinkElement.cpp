#include "Krystal.HTML/HTML/HTMLLinkElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLLinkElement::HTMLLinkElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Link)
  {
  }
}
