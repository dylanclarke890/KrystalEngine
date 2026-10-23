#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
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