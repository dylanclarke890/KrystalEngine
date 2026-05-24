#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  HTMLElement::HTMLElement(Document &document, HTMLTagName tag, NodeFlag flags) noexcept
      : Element(document, flags | NodeFlag::IsHTMLElement), _tagName(tag)
  {
  }

  HTMLElement::HTMLElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Unknown)
  {
  }
}