#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"

namespace Krys::HTML
{
  ContainerNode::ContainerNode(Document &document, NodeType type, NodeFlag flags) noexcept
      : Node(document, type, flags | NodeFlag::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }

#pragma region Node

  ExceptionOr<void> ContainerNode::InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept
  {
    return TreeMutationAlgorithms::PreInsert(newChild, *this, refChild.get());
  }

  ExceptionOr<void> ContainerNode::ReplaceChild(Node &newChild, Node &oldChild) noexcept
  {
    if (&newChild == &oldChild)
    {
      return {};
    }

    if (auto result = InsertBefore(newChild, ShareRefPtr<Node>(&oldChild)); result.HasException())
    {
      return {result.ReleaseException()};
    }

    if (auto result = RemoveChild(oldChild); result.HasException())
    {
      return {result.ReleaseException()};
    }

    return {};
  }

  ExceptionOr<void> ContainerNode::RemoveChild(Node &oldChild) noexcept
  {
    return TreeMutationAlgorithms::PreRemove(oldChild, *this);
  }

  ExceptionOr<void> ContainerNode::AppendChild(Node &newChild) noexcept
  {
    return TreeMutationAlgorithms::PreInsert(newChild, *this, nullptr);
  }

  size_t ContainerNode::CountChildNodes() const noexcept
  {
    return TreeQueries::ChildNodeCount(*this);
  }

#pragma endregion

  void ContainerNode::InvalidateChildNodes()
  {
    if (_nodeRareData)
    {
      _nodeRareData->InvalidateChildNodes();
    }
  }
}
