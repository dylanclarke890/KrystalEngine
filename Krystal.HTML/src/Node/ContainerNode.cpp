#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Document/Document.hpp"

namespace Krys::HTML
{
  ContainerNode::ContainerNode(Document &document, NodeType type, NodeFlags flags) noexcept
      : Node(document, type, flags | NodeFlags::IsContainerNode)
  {
  }

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

  /// @see https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity
  ExceptionOr<void> ContainerNode::EnsurePreInsertValidity(ContainerNode &parent, Node &newChild,
                                                           RefPtr<Node> &&refChild) noexcept
  {
    // The following checks are ordered as they appear in the spec. We can change this later when we optimize.

    // 1.
    if (!parent.IsDocumentNode() && !parent.IsDocumentFragmentNode() && !parent.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 2.
    if (IsHostIncludingAncestorOf(newChild, parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 3.
    if (refChild && refChild->ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    // 4.
    if (!newChild.IsDocumentFragmentNode() && !newChild.IsDocumentTypeNode() && !newChild.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 5.
    if ((newChild.IsTextNode() && parent.IsDocumentNode())
        || (newChild.IsDocumentTypeNode() && !parent.IsDocumentNode()))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 6.
    if (parent.IsDocumentNode())
    {
      if (newChild.IsDocumentFragmentNode())
      {
      }
      else if (newChild.IsElementNode())
      {
      }
      else if (newChild.IsDocumentTypeNode())
      {
      }
    }
  }

  bool ContainerNode::IsHostIncludingAncestorOf(Node &node, Node &other) noexcept
  {
    return false;
  }
}