#include <utility>

#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo
{
  DocumentType::DocumentType(Document &document, DOMString name, DOMString publicId,
                             DOMString systemId) noexcept
      // 1. Let doctype be the result of creating a node that implements DocumentType, given document.
      : Node(document, dom::NodeType::DOCUMENT_TYPE_NODE, dom::NodeFlags::None),
        // 2. Set doctype's name to name, public ID to publicId, and system ID to systemId.
        _name(krys::move(name)), _publicId(krys::move(publicId)), _systemId(krys::move(systemId))
  {
    // 3. Return doctype.
  }

#pragma region ChildNode

  ExceptionOr<void> DocumentType::Before(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> DocumentType::After(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> DocumentType::ReplaceWith(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> DocumentType::Remove() noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::Remove(*this);
  }

#pragma endregion
}
