#include "Krystal.Booey/HTML/HTMLObjectElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLObjectElement::HTMLObjectElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Object)
  {
  }
#pragma region HTMLObjectElement

  dom::ExceptionOr<USVString> HTMLObjectElement::Data() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"data");
  }

  dom::ExceptionOr<void> HTMLObjectElement::Data(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"data", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLObjectElement::Type(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLObjectElement::Name(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLObjectElement::Width(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLObjectElement::Height(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"height", krys::move(value));
  }

#pragma endregion

#pragma region HTMLObjectElement Obsolete members

  dom::DOMString HTMLObjectElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLObjectElement::Align(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Archive() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"archive").value_or(u8"");
  }

  void HTMLObjectElement::Archive(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"archive", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Code() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"code").value_or(u8"");
  }

  void HTMLObjectElement::Code(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"code", krys::move(value));
  }

  bool HTMLObjectElement::Declare() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"declare");
  }

  void HTMLObjectElement::Declare(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"declare", krys::move(value));
  }

  uint32 HTMLObjectElement::HSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLObjectElement::HSpace(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"hspace", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Standby() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"standby").value_or(u8"");
  }

  void HTMLObjectElement::Standby(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"standby", krys::move(value));
  }

  uint32 HTMLObjectElement::VSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLObjectElement::VSpace(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"vspace", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::CodeBase() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"codebase").value_or(u8"");
  }

  void HTMLObjectElement::CodeBase(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"codebase", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::CodeType() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"codetype").value_or(u8"");
  }

  void HTMLObjectElement::CodeType(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"codetype", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::UseMap() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLObjectElement::UseMap(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"usemap", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Border() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLObjectElement::Border(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"border", krys::move(value));
  }

#pragma endregion
}
