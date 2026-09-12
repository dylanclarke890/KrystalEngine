#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLSlotElement::HTMLSlotElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Slot, HTMLElementFlags::None, NodeFlags::IsHTMLSlotElement)
  {
  }

#pragma region HTMLSlotElement

  dom::DOMString HTMLSlotElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSlotElement::Name(dom::DOMString &&name) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(name));
  }

#pragma endregion
}
