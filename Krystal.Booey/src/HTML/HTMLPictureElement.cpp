#include "Krystal.Booey/HTML/HTMLPictureElement.hpp"

namespace krys::boo::html
{
  HTMLPictureElement::HTMLPictureElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Picture)
  {
  }
}
