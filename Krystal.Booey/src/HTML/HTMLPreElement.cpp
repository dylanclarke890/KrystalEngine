#include "Krystal.Booey/HTML/HTMLPreElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLPreElement::HTMLPreElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Pre)
  {
  }

#pragma region HTMLPreElement Obsolete members

  int32 HTMLPreElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<int32>(*this, u8"width");
  }

  void HTMLPreElement::Width(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"width", krys::move(value));
  }

#pragma endregion
}