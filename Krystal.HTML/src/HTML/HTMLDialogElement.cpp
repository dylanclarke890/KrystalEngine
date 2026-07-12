#include "Krystal.HTML/HTML/HTMLDialogElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLDialogElement::HTMLDialogElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Dialog)
  {
  }

#pragma region HTMLDialogElement

  bool HTMLDialogElement::Open() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"open");
  }

  void HTMLDialogElement::Open(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"open", Krys::Move(value));
  }

  DOMString HTMLDialogElement::ReturnValue() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"returnvalue").value_or(u8"");
  }

  void HTMLDialogElement::ReturnValue(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"returnvalue", Krys::Move(value));
  }

  DOMString HTMLDialogElement::ClosedBy() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"closedby").value_or(u8"");
  }

  void HTMLDialogElement::ClosedBy(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"closedby", Krys::Move(value));
  }

#pragma endregion
}
