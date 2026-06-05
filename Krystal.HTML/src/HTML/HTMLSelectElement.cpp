#include "Krystal.HTML/HTML/HTMLSelectElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLSelectElement::HTMLSelectElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Select)
  {
  }

#pragma region HTMLSelectElement

#pragma endregion
}