#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Tree/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"

namespace Krys::HTML
{
  ContainerNode::ContainerNode(Document &document, NodeType type, NodeFlag flags) noexcept
      : Node(document, type, flags | NodeFlag::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }

#pragma region Node

  ExceptionOr<void> ContainerNode::InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept
  {
    if (auto result = TreeMutationAlgorithms::PreInsert(newChild, *this, refChild.get());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return TreeMutationAlgorithms::Insert(newChild, *this, refChild.get(), SuppressObservers(false));
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
    if (auto result = TreeMutationAlgorithms::PreRemove(oldChild, *this); result.HasException())
    {
      return result.ReleaseException();
    }

    return TreeMutationAlgorithms::Remove(oldChild, *this, SuppressObservers(false));
  }

  ExceptionOr<void> ContainerNode::AppendChild(Node &newChild) noexcept
  {
    if (auto result = TreeMutationAlgorithms::PreInsert(newChild, *this, nullptr); result.HasException())
    {
      return result.ReleaseException();
    }

    return TreeMutationAlgorithms::Insert(newChild, *this, nullptr, SuppressObservers(false));
  }

#pragma endregion

  void ContainerNode::OnChildrenChanged() noexcept
  {
    if (_nodeRareData)
    {
      _nodeRareData->InvalidateChildNodes();
    }
  }
}
