#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLElement::HTMLElement(Document &document, HTMLTagName tag, NodeFlags flags) noexcept
      : Element(document, flags | NodeFlags::IsHTMLElement), _tagName(tag)
  {
  }

  HTMLElement::HTMLElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Unknown)
  {
  }
}