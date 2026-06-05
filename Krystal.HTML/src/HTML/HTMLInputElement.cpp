#include "Krystal.HTML/HTML/HTMLInputElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLInputElement::HTMLInputElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Input)
  {
  }

#pragma region HTMLInputElement

  DOMString HTMLInputElement::Accept() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"accept").value_or(u8"");
  }

  void HTMLInputElement::Accept(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"accept", Krys::Move(value));
  }

  bool HTMLInputElement::Alpha() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"alpha");
  }

  void HTMLInputElement::Alpha(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"alpha", Krys::Move(value));
  }

  DOMString HTMLInputElement::Alt() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLInputElement::Alt(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"alt", Krys::Move(value));
  }

  DOMString HTMLInputElement::Autocomplete() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLInputElement::Autocomplete(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"autocomplete", Krys::Move(value));
  }

  bool HTMLInputElement::DefaultChecked() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"checked");
  }

  void HTMLInputElement::DefaultChecked(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"checked", Krys::Move(value));
  }

  bool HTMLInputElement::Checked() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"checked");
  }

  void HTMLInputElement::Checked(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"checked", Krys::Move(value));
  }

  DOMString HTMLInputElement::ColorSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"colorspace").value_or(u8"");
  }

  void HTMLInputElement::ColorSpace(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"colorspace", Krys::Move(value));
  }

  DOMString HTMLInputElement::DirName() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"dirname").value_or(u8"");
  }

  void HTMLInputElement::DirName(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"dirname", Krys::Move(value));
  }

  bool HTMLInputElement::Disabled() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLInputElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLInputElement::FormAction() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"formaction");
  }

  ExceptionOr<void> HTMLInputElement::FormAction(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"formaction", Krys::Move(value));
  }

  DOMString HTMLInputElement::FormEnctype() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"formenctype").value_or(u8"");
  }

  void HTMLInputElement::FormEnctype(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"formenctype", Krys::Move(value));
  }

  DOMString HTMLInputElement::FormMethod() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"formmethod").value_or(u8"");
  }

  void HTMLInputElement::FormMethod(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"formmethod", Krys::Move(value));
  }

  bool HTMLInputElement::FormNoValidate() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"formnovalidate");
  }

  void HTMLInputElement::FormNoValidate(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"formnovalidate", Krys::Move(value));
  }

  DOMString HTMLInputElement::FormTarget() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"formtarget").value_or(u8"");
  }

  void HTMLInputElement::FormTarget(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"formtarget", Krys::Move(value));
  }

  uint32 HTMLInputElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLInputElement::Height(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"height", Krys::Move(value));
  }

  bool HTMLInputElement::Indeterminate() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"indeterminate");
  }

  void HTMLInputElement::Indeterminate(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"indeterminate", Krys::Move(value));
  }

  DOMString HTMLInputElement::Max() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"max").value_or(u8"");
  }

  void HTMLInputElement::Max(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"max", Krys::Move(value));
  }

  int32 HTMLInputElement::MaxLength() const noexcept
  {
    return Attributes::Reflection::Reflect<int32>(*this, u8"maxlength");
  }

  void HTMLInputElement::MaxLength(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"maxlength", Krys::Move(value));
  }

  DOMString HTMLInputElement::Min() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"min").value_or(u8"");
  }

  void HTMLInputElement::Min(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"min", Krys::Move(value));
  }

  int32 HTMLInputElement::MinLength() const noexcept
  {
    return Attributes::Reflection::Reflect<int32>(*this, u8"minlength");
  }

  void HTMLInputElement::MinLength(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"minlength", Krys::Move(value));
  }

  bool HTMLInputElement::Multiple() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"multiple");
  }

  void HTMLInputElement::Multiple(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"multiple", Krys::Move(value));
  }

  DOMString HTMLInputElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLInputElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  DOMString HTMLInputElement::Pattern() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"pattern").value_or(u8"");
  }

  void HTMLInputElement::Pattern(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"pattern", Krys::Move(value));
  }

  DOMString HTMLInputElement::Placeholder() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"placeholder").value_or(u8"");
  }

  void HTMLInputElement::Placeholder(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"placeholder", Krys::Move(value));
  }

  bool HTMLInputElement::ReadOnly() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"readonly");
  }

  void HTMLInputElement::ReadOnly(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"readonly", Krys::Move(value));
  }

  bool HTMLInputElement::Required() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLInputElement::Required(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"required", Krys::Move(value));
  }

  uint32 HTMLInputElement::Size() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"size");
  }

  void HTMLInputElement::Size(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"size", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLInputElement::Src() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src");
  }

  ExceptionOr<void> HTMLInputElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src", Krys::Move(value));
  }

  DOMString HTMLInputElement::Step() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"step").value_or(u8"");
  }

  void HTMLInputElement::Step(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"step", Krys::Move(value));
  }

  DOMString HTMLInputElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLInputElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

  DOMString HTMLInputElement::DefaultValue() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"defaultvalue").value_or(u8"");
  }

  void HTMLInputElement::DefaultValue(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"defaultvalue", Krys::Move(value));
  }

  DOMString HTMLInputElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLInputElement::Value(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"value", Krys::Move(value));
  }

  uint32 HTMLInputElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLInputElement::Width(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"width", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLInputElement Obsolete members

  DOMString HTMLInputElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLInputElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLInputElement::UseMap() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLInputElement::UseMap(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"usemap", Krys::Move(value));
  }

#pragma endregion
}