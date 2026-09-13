#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLStyleElement::HTMLStyleElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Style)
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
    return Reflection::Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLStyleElement::Media(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

  dom::DOMTokenList &HTMLStyleElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking");
    }

    return *_blocking;
  }

#pragma endregion

#pragma region HTMLStyleElement Obsolete members

  dom::DOMString HTMLStyleElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLStyleElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion
}
