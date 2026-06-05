#include "Krystal.HTML/HTML/HTMLProgressElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLProgressElement::HTMLProgressElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Progress)
  {
  }

#pragma region HTMLProgressElement

  double HTMLProgressElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"value");
  }

  void HTMLProgressElement::Value(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"value", Krys::Move(value));
  }

  double HTMLProgressElement::Max() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"max");
  }

  void HTMLProgressElement::Max(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"max", Krys::Move(value));
  }

#pragma endregion

}