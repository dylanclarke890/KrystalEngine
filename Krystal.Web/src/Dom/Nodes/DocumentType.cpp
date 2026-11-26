#include <utility>

#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Dom/Mixins/ChildNode.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web
{
  DocumentType::DocumentType(Document &document, DOMString name, DOMString publicId,
                             DOMString systemId) noexcept
      // 1. Let doctype be the result of creating a node that implements DocumentType, given document.
      : Node(document, dom::NodeType::DOCUMENT_TYPE_NODE, {}),
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
