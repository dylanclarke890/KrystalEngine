#include "Krystal.HTML/HTML/HTMLMeterElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLMeterElement::HTMLMeterElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Meter)
  {
  }

#pragma region HTMLMeterElement

  double HTMLMeterElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"value");
  }

  void HTMLMeterElement::Value(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"value", Krys::Move(value));
  }

  double HTMLMeterElement::Min() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"min");
  }

  void HTMLMeterElement::Min(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"min", Krys::Move(value));
  }

  double HTMLMeterElement::Max() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"max");
  }

  void HTMLMeterElement::Max(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"max", Krys::Move(value));
  }

  double HTMLMeterElement::Low() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"low");
  }

  void HTMLMeterElement::Low(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"low", Krys::Move(value));
  }

  double HTMLMeterElement::High() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"high");
  }

  void HTMLMeterElement::High(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"high", Krys::Move(value));
  }

  double HTMLMeterElement::Optimum() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"optimum");
  }

  void HTMLMeterElement::Optimum(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"optimum", Krys::Move(value));
  }

#pragma endregion
}