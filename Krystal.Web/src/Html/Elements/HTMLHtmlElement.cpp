#include "Krystal.Web/Html/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"

namespace krys::web
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
