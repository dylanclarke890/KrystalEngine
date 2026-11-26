#include "Krystal.Web/Html/Elements/HTMLDataElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"

namespace krys::web
{
  HTMLDataElement::HTMLDataElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Data)
  {
  }

#pragma region HTMLDataElement

  DOMString HTMLDataElement::Value() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLDataElement::Value(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"value", krys::move(value));
  }

#pragma endregion
}
