#include "Krystal.HTML/Document/DocumentType.hpp"

namespace Krys::HTML
{
  DocumentType::DocumentType(Document &document, const DOMString &name, const DOMString &publicId,
                             const DOMString &systemId) noexcept
      : Node(document, NodeType::DOCUMENT_TYPE_NODE, NodeFlags::None), _name(name), _publicId(publicId),
        _systemId(systemId)
  {
  }

  DOMString DocumentType::NodeName() const noexcept
  {
    return _name;
  }
}