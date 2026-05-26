#include "Krystal.HTML/HTMLElement/HTMLHeadElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  HTMLHeadElement::HTMLHeadElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Head)
  {
  }
}
