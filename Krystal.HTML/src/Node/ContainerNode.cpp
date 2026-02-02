#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Document/Document.hpp"

namespace Krys::HTML
{
  ContainerNode::ContainerNode(Document &document, NodeType type, NodeFlag flags) noexcept
      : Node(document, type, flags | NodeFlag::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }

  ExceptionOr<void> ContainerNode::InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept
  {
    return {};
  }

  ExceptionOr<void> ContainerNode::ReplaceChild(Node &newChild, Node &oldChild) noexcept
  {
    return {};
  }

  ExceptionOr<void> ContainerNode::RemoveChild(Node &oldChild) noexcept
  {
    return {};
  }

  ExceptionOr<void> ContainerNode::AppendChild(Node &newChild) noexcept
  {
    return {};
  }

  /// @see https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity
  ExceptionOr<void> ContainerNode::EnsurePreInsertValidity(Node &node, ContainerNode &parent,
                                                           RefPtr<Node> &&refChild) noexcept
  {
    // The following checks are ordered as they appear in the spec. We can change this later when we optimize.

    // 1.
    if (!parent.IsDocumentNode() && !parent.IsDocumentFragmentNode() && !parent.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 2.
    if (IsHostIncludingAncestorOf(node, parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 3.
    if (refChild != nullptr && refChild->ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    // 4.
    if (!node.IsDocumentFragmentNode() && !node.IsDocumentTypeNode() && !node.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 5.
    if ((node.IsTextNode() && parent.IsDocumentNode())
        || (node.IsDocumentTypeNode() && !parent.IsDocumentNode()))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 6.
    if (parent.IsDocumentNode())
    {
      if (node.IsDocumentFragmentNode())
      {
      }
      else if (node.IsElementNode())
      {
      }
      else if (node.IsDocumentTypeNode())
      {
      }
    }

    return {};
  }

  bool ContainerNode::IsHostIncludingAncestorOf(Node &node, Node &other) noexcept
  {
    return false;
  }
}