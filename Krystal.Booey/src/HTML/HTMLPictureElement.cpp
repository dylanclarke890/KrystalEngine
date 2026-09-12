#include "Krystal.Booey/HTML/HTMLPictureElement.hpp"

namespace krys::boo::html
{
  HTMLPictureElement::HTMLPictureElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Picture)
  {
  }
}
