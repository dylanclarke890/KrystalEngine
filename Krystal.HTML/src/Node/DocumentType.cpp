#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ChildNodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  DocumentType::DocumentType(Document &document, const DOMString &name, const DOMString &publicId,
                             const DOMString &systemId) noexcept
      : Node(document, NodeType::DOCUMENT_TYPE_NODE, NodeFlag::None), _name(name), _publicId(publicId),
        _systemId(systemId)
  {
  }

#pragma region ChildNode

  ExceptionOr<void> DocumentType::Before(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::Before(*this, nodes);
  }

  ExceptionOr<void> DocumentType::After(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::After(*this, nodes);
  }

  ExceptionOr<void> DocumentType::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> DocumentType::Remove() noexcept
  {
    return ChildNodeAlgorithms::Remove(*this);
  }

#pragma endregion
}