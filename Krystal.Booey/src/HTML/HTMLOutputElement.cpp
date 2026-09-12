#include "Krystal.Booey/HTML/HTMLOutputElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLOutputElement::HTMLOutputElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Output)
  {
  }

#pragma region HTMLOutputElement

  DOMTokenList &HTMLOutputElement::HtmlFor() noexcept
  {
    if (!_htmlFor)
    {
      _htmlFor = ElementAlgorithms::CreateDOMTokenList(*this, u8"for");
    }

    return *_htmlFor;
  }

  dom::DOMString HTMLOutputElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLOutputElement::Name(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect(*this, u8"name", krys::move(value));
  }

#pragma endregion
}