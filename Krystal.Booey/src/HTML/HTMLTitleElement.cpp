#include "Krystal.Booey/HTML/HTMLTitleElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"

namespace krys::boo::html
{
  HTMLTitleElement::HTMLTitleElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Title)
  {
  }

#pragma region HTMLTitleElement

  dom::DOMString HTMLTitleElement::Text() const noexcept
  {
    return TextAlgorithms::ChildTextContent(*this);
  }

  dom::ExceptionOr<void> HTMLTitleElement::Text(dom::DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(std::move(value), *this);
  }

#pragma endregion
}
