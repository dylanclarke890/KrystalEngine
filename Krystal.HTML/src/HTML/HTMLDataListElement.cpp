#include "Krystal.HTML/HTML/HTMLDataListElement.hpp"

namespace Krys::HTML
{
  HTMLDataListElement::HTMLDataListElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::DataList)
  {
  }
}