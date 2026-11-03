#include "Krystal.Booey/DOM/Nodes/CDATASection.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  CDATASection::CDATASection(Document &document, DOMString data) noexcept
      : Text(document, krys::move(data), Text::CDATAConstructorTag {})
  {
    // 1. Let node be the result of creating a node that implements CDATASection, given document.
    // 2. Set node's data to data.
    // 3. Return node.
  }
}