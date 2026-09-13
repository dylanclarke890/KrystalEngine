#include "Krystal.Booey/HTML/HTMLDataListElement.hpp"

namespace krys::boo::html
{
  HTMLDataListElement::HTMLDataListElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::DataList)
  {
  }
}