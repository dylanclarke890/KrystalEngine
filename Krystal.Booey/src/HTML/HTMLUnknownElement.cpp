#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::html
{
  HTMLUnknownElement::HTMLUnknownElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Unknown)
  {
  }
}