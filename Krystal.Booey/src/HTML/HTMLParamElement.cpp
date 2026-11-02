#include "Krystal.Booey/HTML/HTMLParamElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLParamElement::HTMLParamElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Param)
  {
  }

  dom::DOMString HTMLParamElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLParamElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLParamElement::Value() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLParamElement::Value(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

  dom::DOMString HTMLParamElement::Type() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLParamElement::Type(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLParamElement::ValueType() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"valuetype").value_or(u8"");
  }

  void HTMLParamElement::ValueType(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"valuetype", krys::move(value));
  }
}
