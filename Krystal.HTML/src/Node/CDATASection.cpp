#include "Krystal.HTML/Node/CDATASection.hpp"
#include "Krystal.HTML/Document/Document.hpp"

namespace Krys::HTML
{
  CDATASection::CDATASection(Document &document, DOMString &&data) noexcept
      : Text(document, Krys::Move(data), NodeType::CDATA_SECTION_NODE, NodeFlags::None)
  {
  }

  DOMString CDATASection::NodeName() const noexcept
  {
    return u8"#cdata-section";
  }
}