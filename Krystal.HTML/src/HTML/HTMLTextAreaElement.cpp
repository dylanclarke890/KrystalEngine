#include "Krystal.HTML/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTextAreaElement::HTMLTextAreaElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::TextArea)
  {
  }

#pragma region HTMLTextAreaElement

  DOMString HTMLTextAreaElement::Autocomplete() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLTextAreaElement::Autocomplete(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"autocomplete", Krys::Move(value));
  }

  uint32 HTMLTextAreaElement::Cols() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"cols");
  }

  void HTMLTextAreaElement::Cols(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"cols", Krys::Move(value));
  }

  DOMString HTMLTextAreaElement::DirName() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"dirname").value_or(u8"");
  }

  void HTMLTextAreaElement::DirName(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"dirname", Krys::Move(value));
  }

  bool HTMLTextAreaElement::Disabled() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLTextAreaElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", Krys::Move(value));
  }

  int32 HTMLTextAreaElement::MaxLength() const noexcept
  {
    return Attributes::Reflection::Reflect<int32>(*this, u8"maxlength");
  }

  void HTMLTextAreaElement::MaxLength(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"maxlength", Krys::Move(value));
  }

  int32 HTMLTextAreaElement::MinLength() const noexcept
  {
    return Attributes::Reflection::Reflect<int32>(*this, u8"minlength");
  }

  void HTMLTextAreaElement::MinLength(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"minlength", Krys::Move(value));
  }

  DOMString HTMLTextAreaElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLTextAreaElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  DOMString HTMLTextAreaElement::Placeholder() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"placeholder").value_or(u8"");
  }

  void HTMLTextAreaElement::Placeholder(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"placeholder", Krys::Move(value));
  }

  bool HTMLTextAreaElement::ReadOnly() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"readonly");
  }

  void HTMLTextAreaElement::ReadOnly(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"readonly", Krys::Move(value));
  }

  bool HTMLTextAreaElement::Required() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLTextAreaElement::Required(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"required", Krys::Move(value));
  }

  uint32 HTMLTextAreaElement::Rows() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"rows");
  }

  void HTMLTextAreaElement::Rows(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"rows", Krys::Move(value));
  }

  DOMString HTMLTextAreaElement::Wrap() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"wrap").value_or(u8"");
  }

  void HTMLTextAreaElement::Wrap(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"wrap", Krys::Move(value));
  }

#pragma endregion
}