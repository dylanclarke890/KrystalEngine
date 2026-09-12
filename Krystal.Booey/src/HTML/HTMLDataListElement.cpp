#include "Krystal.Booey/HTML/HTMLDataListElement.hpp"

namespace krys::boo::html
{
  HTMLDataListElement::HTMLDataListElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::DataList)
  {
  }
}