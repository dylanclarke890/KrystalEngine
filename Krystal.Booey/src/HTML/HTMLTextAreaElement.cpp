#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLTextAreaElement::HTMLTextAreaElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::TextArea)
  {
  }

#pragma region HTMLTextAreaElement

  dom::DOMString HTMLTextAreaElement::Autocomplete() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLTextAreaElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  uint32 HTMLTextAreaElement::Cols() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"cols");
  }

  void HTMLTextAreaElement::Cols(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"cols", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::DirName() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"dirname").value_or(u8"");
  }

  void HTMLTextAreaElement::DirName(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"dirname", krys::move(value));
  }

  bool HTMLTextAreaElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLTextAreaElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  int32 HTMLTextAreaElement::MaxLength() const noexcept
  {
    return Reflection::Reflect<int32>(*this, u8"maxlength");
  }

  void HTMLTextAreaElement::MaxLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"maxlength", krys::move(value));
  }

  int32 HTMLTextAreaElement::MinLength() const noexcept
  {
    return Reflection::Reflect<int32>(*this, u8"minlength");
  }

  void HTMLTextAreaElement::MinLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"minlength", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLTextAreaElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::Placeholder() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"placeholder").value_or(u8"");
  }

  void HTMLTextAreaElement::Placeholder(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"placeholder", krys::move(value));
  }

  bool HTMLTextAreaElement::ReadOnly() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"readonly");
  }

  void HTMLTextAreaElement::ReadOnly(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"readonly", krys::move(value));
  }

  bool HTMLTextAreaElement::Required() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLTextAreaElement::Required(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  uint32 HTMLTextAreaElement::Rows() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"rows");
  }

  void HTMLTextAreaElement::Rows(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"rows", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::Wrap() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"wrap").value_or(u8"");
  }

  void HTMLTextAreaElement::Wrap(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"wrap", krys::move(value));
  }

#pragma endregion
}