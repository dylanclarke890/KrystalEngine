#include "Krystal.Web/Dom/Nodes/CDATASection.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web
{
  CDATASection::CDATASection(Document &document, DOMString data) noexcept
      : Text(document, krys::move(data), Text::CDATAConstructorTag {})
  {
    // 1. Let node be the result of creating a node that implements CDATASection, given document.
    // 2. Set node's data to data.
    // 3. Return node.
  }
}
