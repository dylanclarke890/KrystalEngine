#include "Krystal.HTML/HTML/HTMLSelectedContentElement.hpp"

namespace Krys::HTML
{
  HTMLSelectedContentElement::HTMLSelectedContentElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::SelectedContent)
  {
  }
}