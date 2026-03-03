#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Tree/TreeMutationAlgorithms.hpp"

namespace Krys::HTML
{
  DocumentType::DocumentType(Document &document, const DOMString &name, const DOMString &publicId,
                             const DOMString &systemId) noexcept
      : Node(document, NodeType::DOCUMENT_TYPE_NODE, NodeFlag::None), _name(name), _publicId(publicId),
        _systemId(systemId)
  {
  }

#pragma region ChildNode

  ExceptionOr<void> DocumentType::Before(SmallList<NodeOrString> &&nodes) noexcept
  {
    return {};
  }

  ExceptionOr<void> DocumentType::After(SmallList<NodeOrString> &&nodes) noexcept
  {
    return {};
  }

  ExceptionOr<void> DocumentType::ReplaceWith(SmallList<NodeOrString> &&nodes) noexcept
  {
    return {};
  }

  ExceptionOr<void> DocumentType::Remove() noexcept
  {
    if (auto parent = ShareRefPtr(ParentNode()))
    {
      return TreeMutationAlgorithms::Remove(*this, *parent, SuppressObservers(false));
    }

    return {};
  }

#pragma endregion
}