#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Mixins/ChildNode.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  DocumentType::DocumentType(Document &document, const DOMString &name, const DOMString &publicId,
                             const DOMString &systemId) noexcept
      : Node(document, NodeType::DOCUMENT_TYPE_NODE, NodeFlags::None), _name(name), _publicId(publicId),
        _systemId(systemId)
  {
  }

#pragma region ChildNode

  ExceptionOr<void> DocumentType::Before(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> DocumentType::After(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> DocumentType::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> DocumentType::Remove() noexcept
  {
    return Mixins::ChildNode::Remove(*this);
  }

#pragma endregion
}