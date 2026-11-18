#include "Krystal.Booey/HTML/Elements/HTMLDataElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo
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
