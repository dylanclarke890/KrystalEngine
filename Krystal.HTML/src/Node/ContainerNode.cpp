#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  ContainerNode::ContainerNode(Document &document, HTML::NodeType type, NodeFlag flags) noexcept
      : Node(document, type, flags | NodeFlag::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }

  ExceptionOr<Node &> ContainerNode::InsertBefore(Node &newChild, RawPtr<Node> refChild) noexcept
  {
    return MutationAlgorithms::PreInsert(newChild, *this, refChild);
  }

  ExceptionOr<Node &> ContainerNode::ReplaceChild(Node &newChild, Node &oldChild) noexcept
  {
    return MutationAlgorithms::Replace(oldChild, newChild, *this);
  }

  ExceptionOr<Node &> ContainerNode::RemoveChild(Node &oldChild) noexcept
  {
    return MutationAlgorithms::PreRemove(oldChild, *this);
  }

  ExceptionOr<Node &> ContainerNode::AppendChild(Node &newChild) noexcept
  {
    return MutationAlgorithms::Append(newChild, *this);
  }
}
