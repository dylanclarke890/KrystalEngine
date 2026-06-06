#include "Krystal.HTML/HTML/HTMLBaseElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLBaseElement::HTMLBaseElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Base)
  {
  }

#pragma region HTMLBaseElement

  ExceptionOr<USVString> HTMLBaseElement::Href() const noexcept
  {
    // The href IDL attribute, on getting, must return the result of running the following algorithm:
    // Let document be element's node document.
    // Let url be the value of the href attribute of this element, if it has one, and the empty string
    // otherwise.
    // Let urlRecord be the result of parsing url with document's fallback base URL, and document's character
    // encoding. (Thus, the base element isn't affected by other base elements or itself.)
    // If urlRecord is failure, return url.
    // Return the serialization of urlRecord.
    
    return Attributes::Reflection::Reflect<USVString>(*this, u8"href");
  }

  ExceptionOr<void> HTMLBaseElement::Href(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectSetter<USVString>(*this, u8"href", Krys::Move(value));
  }

  DOMString HTMLBaseElement::Target() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLBaseElement::Target(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"target", Krys::Move(value));
  }

#pragma endregion
}
