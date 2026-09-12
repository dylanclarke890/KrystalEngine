#include "Krystal.Booey/HTML/HTMLProgressElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLProgressElement::HTMLProgressElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Progress)
  {
  }

#pragma region HTMLProgressElement

  double HTMLProgressElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"value");
  }

  void HTMLProgressElement::Value(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"value", krys::move(value));
  }

  double HTMLProgressElement::Max() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"max");
  }

  void HTMLProgressElement::Max(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"max", krys::move(value));
  }

#pragma endregion

}