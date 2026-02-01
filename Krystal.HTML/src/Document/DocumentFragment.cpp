#include "Krystal.HTML/Document/DocumentFragment.hpp"

namespace Krys::HTML
{

  DocumentFragment::DocumentFragment(Document &document, NodeFlags flags) noexcept
      : ContainerNode(document, NodeType::DOCUMENT_FRAGMENT_NODE, flags)
  {
  }

  utf8_string DocumentFragment::NodeName() const noexcept
  {
    return u8"#document-fragment";
  }
}