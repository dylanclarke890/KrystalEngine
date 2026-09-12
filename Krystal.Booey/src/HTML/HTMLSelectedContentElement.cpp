#include "Krystal.Booey/HTML/HTMLSelectedContentElement.hpp"

namespace krys::boo::html
{
  HTMLSelectedContentElement::HTMLSelectedContentElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::SelectedContent)
  {
  }
}