#include "Krystal.HTML/DocumentType.hpp"

namespace Krys::HTML
{
  DocumentType::DocumentType(Document &document, const utf8_string &name, const utf8_string &publicId,
                             const utf8_string &systemId) noexcept
      : Node(document, NodeType::DOCUMENT_TYPE_NODE, NodeFlag::None), _name(name), _publicId(publicId),
        _systemId(systemId)
  {
  }

  utf8_string DocumentType::NodeName() const noexcept
  {
    return _name;
  }
}