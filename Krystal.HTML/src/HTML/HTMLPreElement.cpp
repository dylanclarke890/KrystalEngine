#include "Krystal.HTML/HTML/HTMLPreElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLPreElement::HTMLPreElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Pre)
  {
  }

#pragma region HTMLPreElement Obsolete members

  int32 HTMLPreElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<int32>(*this, u8"width");
  }

  void HTMLPreElement::Width(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"width", Krys::Move(value));
  }

#pragma endregion
}