#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLSlotElement::HTMLSlotElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Slot, HTMLElementFlags::None,
                    dom::NodeFlags::IsHTMLSlotElement)
  {
  }

#pragma region HTMLSlotElement

  dom::DOMString HTMLSlotElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSlotElement::Name(dom::DOMString &&name) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(name));
  }

#pragma endregion
}
