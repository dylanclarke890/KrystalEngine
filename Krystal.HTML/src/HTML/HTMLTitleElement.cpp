#include "Krystal.HTML/HTML/HTMLTitleElement.hpp"
#include "Krystal.HTML/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"

namespace Krys::HTML
{
  HTMLTitleElement::HTMLTitleElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Title)
  {
  }

#pragma region HTMLTitleElement

  DOMString HTMLTitleElement::Text() const noexcept
  {
    return TextAlgorithms::ChildTextContent(*this);
  }

  ExceptionOr<void> HTMLTitleElement::Text(DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(std::move(value), *this);
  }

#pragma endregion
}
