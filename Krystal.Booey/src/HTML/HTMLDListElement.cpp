#include "Krystal.Booey/HTML/HTMLDListElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLDListElement::HTMLDListElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::DList)
  {
  }

#pragma region HTMLDListElement Obsolete members

  bool HTMLDListElement::Compact() const noexcept
  {
    return Reflect<bool>(*this, u8"compact");
  }

  void HTMLDListElement::Compact(bool value) noexcept
  {
    Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion
}