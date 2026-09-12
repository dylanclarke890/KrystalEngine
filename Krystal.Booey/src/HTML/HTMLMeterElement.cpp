#include "Krystal.Booey/HTML/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLMeterElement::HTMLMeterElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Meter)
  {
  }

#pragma region HTMLMeterElement

  double HTMLMeterElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"value");
  }

  void HTMLMeterElement::Value(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"value", krys::move(value));
  }

  double HTMLMeterElement::Min() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"min");
  }

  void HTMLMeterElement::Min(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"min", krys::move(value));
  }

  double HTMLMeterElement::Max() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"max");
  }

  void HTMLMeterElement::Max(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"max", krys::move(value));
  }

  double HTMLMeterElement::Low() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"low");
  }

  void HTMLMeterElement::Low(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"low", krys::move(value));
  }

  double HTMLMeterElement::High() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"high");
  }

  void HTMLMeterElement::High(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"high", krys::move(value));
  }

  double HTMLMeterElement::Optimum() const noexcept
  {
    return Attributes::Reflection::Reflect<double>(*this, u8"optimum");
  }

  void HTMLMeterElement::Optimum(double value) noexcept
  {
    Attributes::Reflection::Reflect<double>(*this, u8"optimum", krys::move(value));
  }

#pragma endregion
}