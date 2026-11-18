#include "Krystal.Booey/HTML/Elements/HTMLParamElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLParamElement::HTMLParamElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Param)
  {
  }

  DOMString HTMLParamElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLParamElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  DOMString HTMLParamElement::Value() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLParamElement::Value(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"value", krys::move(value));
  }

  DOMString HTMLParamElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLParamElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

  DOMString HTMLParamElement::ValueType() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"valuetype").value_or(u8"");
  }

  void HTMLParamElement::ValueType(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"valuetype", krys::move(value));
  }
}
