#include "Krystal.HTML/HTML/HTMLMediaElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLMediaElement::HTMLMediaElement(Document &document, DOMInterface interface) noexcept
      : HTMLElement(document, interface, HTMLElementFlags::IsMediaElement)
  {
  }

  HTMLMediaElement::HTMLMediaElement(Document &document) noexcept
      : HTMLMediaElement(document, DOMInterface::Media)
  {
  }
}
