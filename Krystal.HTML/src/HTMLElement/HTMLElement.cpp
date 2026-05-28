#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"

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