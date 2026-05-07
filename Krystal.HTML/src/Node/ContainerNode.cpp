#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"

namespace Krys::HTML
{
  ContainerNode::ContainerNode(Document &document, HTML::NodeType type, NodeFlag flags) noexcept
      : Node(document, type, flags | NodeFlag::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }

#pragma region Node

  ExceptionOr<Node &> ContainerNode::InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept
  {
    return MutationAlgorithms::PreInsert(newChild, *this, refChild.get());
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

  size_t ContainerNode::CountChildNodes() const noexcept
  {
    return TreeQueries::ChildNodeCount(*this);
  }

#pragma endregion
}
