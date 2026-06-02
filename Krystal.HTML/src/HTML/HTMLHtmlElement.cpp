#include "Krystal.HTML/HTML/HTMLHtmlElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLHtmlElement::HTMLHtmlElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Html)
  {
  }

#pragma region HTMLHtmlElement Obsolete members

  DOMString HTMLHtmlElement::Version() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"version").value_or(u8"");
  }

  void HTMLHtmlElement::Version(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"version", Krys::Move(value));
  }

#pragma endregion
}
