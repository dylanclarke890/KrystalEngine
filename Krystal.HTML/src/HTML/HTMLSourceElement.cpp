#include "Krystal.HTML/HTML/HTMLSourceElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLSourceElement::HTMLSourceElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Source)
  {
  }
#pragma region HTMLSourceElement

  ExceptionOr<USVString> HTMLSourceElement::Src() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"src");
  }
  
  ExceptionOr<void> HTMLSourceElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"src", Krys::Move(value));
  }
  
  DOMString HTMLSourceElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }
  
  void HTMLSourceElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }
  
  ExceptionOr<USVString> HTMLSourceElement::SrcSet() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"srcset");
  }
  
  ExceptionOr<void> HTMLSourceElement::SrcSet(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"srcset", Krys::Move(value));
  }
  
  DOMString HTMLSourceElement::Sizes() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"sizes").value_or(u8"");
  }
  
  void HTMLSourceElement::Sizes(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"sizes", Krys::Move(value));
  }
  
  DOMString HTMLSourceElement::Media() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"media").value_or(u8"");
  }
  
  void HTMLSourceElement::Media(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"media", Krys::Move(value));
  }
  
  uint32 HTMLSourceElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"width");
  }
  
  void HTMLSourceElement::Width(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"width", Krys::Move(value));
  }
  
  uint32 HTMLSourceElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"height");
  }
  
  void HTMLSourceElement::Height(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"height", Krys::Move(value));
  }

  #pragma endregion
}
