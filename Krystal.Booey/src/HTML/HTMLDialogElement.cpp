#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLDialogElement::HTMLDialogElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Dialog)
  {
  }

#pragma region HTMLDialogElement

  bool HTMLDialogElement::Open() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"open");
  }

  void HTMLDialogElement::Open(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"open", krys::move(value));
  }

  dom::DOMString HTMLDialogElement::ReturnValue() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"returnvalue").value_or(u8"");
  }

  void HTMLDialogElement::ReturnValue(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"returnvalue", krys::move(value));
  }

  dom::DOMString HTMLDialogElement::ClosedBy() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"closedby").value_or(u8"");
  }

  void HTMLDialogElement::ClosedBy(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"closedby", krys::move(value));
  }

#pragma endregion
}
