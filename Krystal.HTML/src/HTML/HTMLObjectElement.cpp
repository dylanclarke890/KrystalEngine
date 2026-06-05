#include "Krystal.HTML/HTML/HTMLObjectElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLObjectElement::HTMLObjectElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Object)
  {
  }
#pragma region HTMLObjectElement

  ExceptionOr<USVString> HTMLObjectElement::Data() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"data");
  }

  ExceptionOr<void> HTMLObjectElement::Data(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"data", Krys::Move(value));
  }

  DOMString HTMLObjectElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLObjectElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

  DOMString HTMLObjectElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLObjectElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  DOMString HTMLObjectElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLObjectElement::Width(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"width", Krys::Move(value));
  }

  DOMString HTMLObjectElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLObjectElement::Height(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"height", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLObjectElement Obsolete members

  DOMString HTMLObjectElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLObjectElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLObjectElement::Archive() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"archive").value_or(u8"");
  }

  void HTMLObjectElement::Archive(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"archive", Krys::Move(value));
  }

  DOMString HTMLObjectElement::Code() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"code").value_or(u8"");
  }

  void HTMLObjectElement::Code(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"code", Krys::Move(value));
  }

  bool HTMLObjectElement::Declare() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"declare");
  }

  void HTMLObjectElement::Declare(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"declare", Krys::Move(value));
  }

  uint32 HTMLObjectElement::HSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLObjectElement::HSpace(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"hspace", Krys::Move(value));
  }

  DOMString HTMLObjectElement::Standby() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"standby").value_or(u8"");
  }

  void HTMLObjectElement::Standby(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"standby", Krys::Move(value));
  }

  uint32 HTMLObjectElement::VSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLObjectElement::VSpace(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"vspace", Krys::Move(value));
  }

  DOMString HTMLObjectElement::CodeBase() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"codebase").value_or(u8"");
  }

  void HTMLObjectElement::CodeBase(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"codebase", Krys::Move(value));
  }

  DOMString HTMLObjectElement::CodeType() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"codetype").value_or(u8"");
  }

  void HTMLObjectElement::CodeType(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"codetype", Krys::Move(value));
  }

  DOMString HTMLObjectElement::UseMap() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLObjectElement::UseMap(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"usemap", Krys::Move(value));
  }

  DOMString HTMLObjectElement::Border() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLObjectElement::Border(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"border", Krys::Move(value));
  }

#pragma endregion
}
