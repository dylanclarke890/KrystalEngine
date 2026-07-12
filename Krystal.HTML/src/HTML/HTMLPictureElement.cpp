#include "Krystal.HTML/HTML/HTMLPictureElement.hpp"

namespace Krys::HTML
{
  HTMLPictureElement::HTMLPictureElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Picture)
  {
  }
}
