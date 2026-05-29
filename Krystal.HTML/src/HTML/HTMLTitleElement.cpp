#include "Krystal.HTML/HTML/HTMLTitleElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"

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
