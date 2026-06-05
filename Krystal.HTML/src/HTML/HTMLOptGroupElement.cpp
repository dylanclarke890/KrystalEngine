#include "Krystal.HTML/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLOptGroupElement::HTMLOptGroupElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::OptGroup)
  {
  }

#pragma region HTMLOptGroupElement

  bool HTMLOptGroupElement::Disabled() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLOptGroupElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", Krys::Move(value));
  }

  DOMString HTMLOptGroupElement::Label() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLOptGroupElement::Label(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"label", Krys::Move(value));
  }

#pragma endregion
}