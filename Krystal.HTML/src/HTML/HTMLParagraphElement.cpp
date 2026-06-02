#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLParagraphElement.hpp"

namespace Krys::HTML
{
  HTMLParagraphElement::HTMLParagraphElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Heading)
  {
  }

#pragma region HTMLHeadingElement Obsolete members

  DOMString HTMLParagraphElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLParagraphElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

#pragma endregion
}