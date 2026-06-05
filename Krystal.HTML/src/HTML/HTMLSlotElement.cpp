#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLSlotElement::HTMLSlotElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Slot, HTMLElementFlags::None, NodeFlags::IsHTMLSlotElement)
  {
  }

#pragma region HTMLSlotElement

  DOMString HTMLSlotElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSlotElement::Name(DOMString &&name) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(name));
  }

#pragma endregion
}