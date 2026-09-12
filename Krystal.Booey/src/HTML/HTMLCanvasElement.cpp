#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLCanvasElement::HTMLCanvasElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Canvas)
  {
  }

#pragma region HTMLCanvasElement

  uint32 HTMLCanvasElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLCanvasElement::Width(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLCanvasElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLCanvasElement::Height(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

#pragma endregion
}