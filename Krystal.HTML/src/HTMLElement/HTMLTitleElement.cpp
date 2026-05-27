#include "Krystal.HTML/HTMLElement/HTMLTitleElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  HTMLTitleElement::HTMLTitleElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Title)
  {
  }

  DOMString HTMLTitleElement::Text() const noexcept
  {
    return TextAlgorithms::ChildTextContent(*this);
  }

  ExceptionOr<void> HTMLTitleElement::Text(DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(std::move(value), *this);
  }
}
