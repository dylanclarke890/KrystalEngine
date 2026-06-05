#include "Krystal.HTML/HTML/HTMLTrackElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTrackElement::HTMLTrackElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Track)
  {
  }

#pragma region HTMLTrackElement

  DOMString HTMLTrackElement::Kind() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"kind").value_or(u8"");
  }

  void HTMLTrackElement::Kind(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"kind", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLTrackElement::Src() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src");
  }

  ExceptionOr<void> HTMLTrackElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src", Krys::Move(value));
  }

  DOMString HTMLTrackElement::SrcLang() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"srclang").value_or(u8"");
  }

  void HTMLTrackElement::SrcLang(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"srclang", Krys::Move(value));
  }

  DOMString HTMLTrackElement::Label() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLTrackElement::Label(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"label", Krys::Move(value));
  }

  bool HTMLTrackElement::Default() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"default");
  }

  void HTMLTrackElement::Default(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"default", Krys::Move(value));
  }

#pragma endregion
}
