#include "Krystal.HTML/Node/CDATASection.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  CDATASection::CDATASection(Document &document, DOMString &&data) noexcept
      : Text(document, Krys::Move(data), NodeType::CDATA_SECTION_NODE, NodeFlag::None)
  {
  }
}