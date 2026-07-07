#include "Krystal.HTML/HTML/HTMLOutputElement.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
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

  DOMString HTMLOutputElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLOutputElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect(*this, u8"name", Krys::Move(value));
  }

#pragma endregion
}