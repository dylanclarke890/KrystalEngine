#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  DocumentType::DocumentType(Document &document, const DOMString &name, const DOMString &publicId,
                             const DOMString &systemId) noexcept
      // 1. Let doctype be the result of creating a node that implements DocumentType, given document.
      : Node(document, NodeType::DOCUMENT_TYPE_NODE, NodeFlags::None),
        // 2. Set doctype's name to name, public ID to publicId, and system ID to systemId.
        _name(name), _publicId(publicId), _systemId(systemId)
  {
    // 3. Return doctype.
  }

#pragma region ChildNode

  ExceptionOr<void> DocumentType::Before(const List<NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> DocumentType::After(const List<NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> DocumentType::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> DocumentType::Remove() noexcept
  {
    html::CEReactionsScope ceReactions;
    return mixins::ChildNode::Remove(*this);
  }

#pragma endregion
}