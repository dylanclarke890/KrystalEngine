#include "Krystal.Booey/HTML/HTMLSourceElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLSourceElement::HTMLSourceElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Source)
  {
  }
#pragma region HTMLSourceElement

  dom::ExceptionOr<USVString> HTMLSourceElement::Src() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"src");
  }
  
  dom::ExceptionOr<void> HTMLSourceElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"src", krys::move(value));
  }
  
  dom::DOMString HTMLSourceElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }
  
  void HTMLSourceElement::Type(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }
  
  dom::ExceptionOr<USVString> HTMLSourceElement::SrcSet() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"srcset");
  }
  
  dom::ExceptionOr<void> HTMLSourceElement::SrcSet(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"srcset", krys::move(value));
  }
  
  dom::DOMString HTMLSourceElement::Sizes() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"sizes").value_or(u8"");
  }
  
  void HTMLSourceElement::Sizes(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"sizes", krys::move(value));
  }
  
  dom::DOMString HTMLSourceElement::Media() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }
  
  void HTMLSourceElement::Media(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }
  
  uint32 HTMLSourceElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"width");
  }
  
  void HTMLSourceElement::Width(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }
  
  uint32 HTMLSourceElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"height");
  }
  
  void HTMLSourceElement::Height(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

  #pragma endregion
}
