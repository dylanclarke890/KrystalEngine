#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLStyleElement::HTMLStyleElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Style)
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

  dom::DOMString HTMLStyleElement::Media() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLStyleElement::Media(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
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

  dom::DOMString HTMLStyleElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLStyleElement::Type(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion
}
