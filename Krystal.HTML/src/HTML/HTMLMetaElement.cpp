#include "Krystal.HTML/HTML/HTMLMetaElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"

namespace Krys::HTML
{
  HTMLMetaElement::HTMLMetaElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Meta)
  {
  }
}
