#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Document/CustomElementRegistry.hpp"

namespace Krys::HTML
{
  Document::Document() noexcept
      : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlags::IsContainerNode), TreeScope(*this)
  {
  }

  DOMString Document::NodeName() const noexcept
  {
    return u8"#document";
  }
}