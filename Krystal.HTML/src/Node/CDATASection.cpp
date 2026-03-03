#include "Krystal.HTML/Node/CDATASection.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  CDATASection::CDATASection(Document &document, DOMString &&data) noexcept
      : Text(document, Krys::Move(data), NodeType::CDATA_SECTION_NODE, NodeFlag::None)
  {
  }

  DOMString CDATASection::NodeName() const noexcept
  {
    return u8"#cdata-section";
  }
}