#include "Krystal.HTML/DOM/CDATASection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"

namespace Krys::HTML
{
  CDATASection::CDATASection(Document &document, DOMString &&data) noexcept
      : Text(document, Krys::Move(data), Text::CDATAConstructorTag {})
  {
  }
}