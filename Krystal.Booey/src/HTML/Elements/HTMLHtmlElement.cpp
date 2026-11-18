#include "Krystal.Booey/HTML/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLHtmlElement::HTMLHtmlElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Html)
  {
  }

#pragma region HTMLHtmlElement Obsolete members

  DOMString HTMLHtmlElement::Version() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"version").value_or(u8"");
  }

  void HTMLHtmlElement::Version(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"version", krys::move(value));
  }

#pragma endregion
}
