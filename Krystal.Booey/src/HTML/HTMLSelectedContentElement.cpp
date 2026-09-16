#include "Krystal.Booey/HTML/HTMLSelectedContentElement.hpp"

namespace krys::boo::html
{
  HTMLSelectedContentElement::HTMLSelectedContentElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::SelectedContent)
  {
  }
}