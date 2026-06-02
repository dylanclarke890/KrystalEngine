#include "Krystal.HTML/HTML/HTMLStyleElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLStyleElement::HTMLStyleElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Style)
  {
  }

#pragma region HTMLStyleElement

  bool HTMLStyleElement::Disabled() const noexcept
  {
    // TODO(HTMLSTYLEELEMENT, HTML): get the disabled state of this' associated stylesheet.
    return false;
  }

  void HTMLStyleElement::Disabled(bool value) noexcept
  {
    // TODO(HTMLSTYLEELEMENT, HTML): set the disabled state of this' associated stylesheet.
  }

  DOMString HTMLStyleElement::Media() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLStyleElement::Media(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"media", Krys::Move(value));
  }

  DOMTokenList &HTMLStyleElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking");
    }

    return *_blocking;
  }

#pragma endregion

#pragma region HTMLStyleElement Obsolete members

  DOMString HTMLStyleElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLStyleElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

#pragma endregion
}
