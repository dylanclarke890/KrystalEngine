#include "Krystal.HTML/Node/ContainerNode.hpp"

namespace Krys::HTML
{
  ExceptionOr<void> ContainerNode::InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept
  {
  }

  ExceptionOr<void> ContainerNode::ReplaceChild(Node &newChild, Node &oldChild) noexcept
  {
  }

  ExceptionOr<void> ContainerNode::RemoveChild(Node &oldChild) noexcept
  {
  }

  ExceptionOr<void> ContainerNode::AppendChild(Node &newChild) noexcept
  {
  }

  ExceptionOr<void> ContainerNode::EnsurePreInsertValidity(ContainerNode &parent, Node &newChild,
                                                           RefPtr<Node> &&refChild) noexcept
  {
    if (!parent.IsDocumentNode() && !parent.IsDocumentFragmentNode() && !parent.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (IsHostIncludingAncestorOf(newChild, parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (refChild && refChild->ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    if (!newChild.IsDocumentFragmentNode() && !newChild.IsElementNode() && !newChild.IsTextNode()
        && !newChild.IsCommentNode() && !newChild.IsProcessingInstructionNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }
  }

  bool ContainerNode::IsHostIncludingAncestorOf(Node &node, Node &other) noexcept
  {
    return false;
  }
}