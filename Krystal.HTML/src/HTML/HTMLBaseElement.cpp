#include "Krystal.HTML/HTML/HTMLBaseElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLBaseElement::HTMLBaseElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Base)
  {
  }

  USVString HTMLBaseElement::Href() const noexcept
  {
    // TODO(URL, HTML): implement the Href() getter method
    return {};
  }

  ExceptionOr<void> HTMLBaseElement::Href(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectSetter<USVString>(*this, u8"target", Krys::Move(value));
  }

  DOMString HTMLBaseElement::Target() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLBaseElement::Target(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"target", Krys::Move(value));
  }
}
