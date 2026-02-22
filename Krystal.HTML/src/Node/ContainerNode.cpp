#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/NodeQueries.hpp"
#include "Krystal.HTML/Node/NodeTraversal.hpp"

namespace Krys::HTML
{
  namespace
  {
    KRYS_NODISCARD bool IsDocTypeOrDocTypeFollows(RawPtr<Node> node) noexcept
    {
      if (node == nullptr)
      {
        return false;
      }

      if (node->IsDocumentTypeNode())
      {
        return true;
      }

      RawPtr<Node> current = node;
      while (current = NodeTraversal::Next(*current))
      {
        if (current->IsDocumentTypeNode())
        {
          return true;
        }
      }

      return false;
    }
  }

  ContainerNode::ContainerNode(Document &document, NodeType type, NodeFlag flags) noexcept
      : Node(document, type, flags | NodeFlag::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }

  ExceptionOr<void> ContainerNode::InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept
  {
    if (auto result = PreInsert(newChild, refChild.get()); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
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
    if (oldChild.ParentNode() != this)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    if (oldChild.PreviousSibling())
    {
      oldChild.PreviousSibling()->SetNextSibling(oldChild.NextSibling());
      oldChild.SetPreviousSibling(nullptr);
    }
    else
    {
      assert(_firstChild == &oldChild);
      _firstChild = oldChild.NextSibling();
    }

    if (oldChild.NextSibling())
    {
      oldChild.NextSibling()->SetPreviousSibling(oldChild.PreviousSibling());
      oldChild.SetNextSibling(nullptr);
    }
    else
    {
      assert(_lastChild == &oldChild);
      _lastChild = oldChild.PreviousSibling();
    }

    assert(_firstChild != &oldChild);
    assert(_lastChild != &oldChild);
    assert(!oldChild.PreviousSibling());
    assert(!oldChild.NextSibling());

    oldChild.SetParentNode(nullptr);
    oldChild.SetTreeScopeRecursively(*OwnerDocument());

    return {};
  }

  ExceptionOr<void> ContainerNode::AppendChild(Node &newChild) noexcept
  {
    if (auto result = PreInsert(newChild, nullptr); result.HasException())
    {
      return {result.ReleaseException()};
    }

    return {};
  }

  /// @see https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity
  ExceptionOr<void> ContainerNode::EnsurePreInsertValidity(Node &node, RawPtr<Node> refChild) noexcept
  {
    auto &parent = *this;

    if (!parent.IsDocumentNode() && !parent.IsDocumentFragmentNode() && !parent.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (NodeQueries::IsHostIncludingAncestorOf(node, parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (refChild && refChild->ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    if (!node.IsDocumentFragmentNode() && !node.IsDocumentTypeNode() && !node.IsElementNode()
        && !node.IsCharacterDataNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if ((node.IsTextNode() && parent.IsDocumentNode())
        || (node.IsDocumentTypeNode() && !parent.IsDocumentNode()))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (parent.IsDocumentNode())
    {
      if (node.IsDocumentFragmentNode())
      {
        uint32 elementCount = 0;
        for (RawPtr<Node> child = node.FirstChild(); child != nullptr; child = child->NextSibling())
        {
          if (child->IsElementNode())
          {
            ++elementCount;
            if (elementCount > 1)
            {
              return Exception {ExceptionCode::HierarchyRequestError};
            }
          }
          else if (child->IsTextNode())
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }
        }

        if (elementCount == 1)
        {
          for (RawPtr<Node> child = parent.FirstChild(); child != nullptr; child = child->NextSibling())
          {
            if (child->IsElementNode())
            {
              return Exception {ExceptionCode::HierarchyRequestError};
            }
          }

          if (IsDocTypeOrDocTypeFollows(refChild))
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }
        }
      }
      else if (node.IsElementNode())
      {
        for (RawPtr<Node> child = parent.FirstChild(); child != nullptr; child = child->NextSibling())
        {
          if (child->IsElementNode())
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }
        }

        if (IsDocTypeOrDocTypeFollows(refChild))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }
      }
      else if (node.IsDocumentTypeNode())
      {
        bool hasElementChild = false;
        for (RawPtr<Node> child = parent.FirstChild(); child != nullptr; child = child->NextSibling())
        {
          if (child->IsDocumentTypeNode())
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }

          hasElementChild = hasElementChild || child->IsElementNode();
        }

        if (refChild != nullptr)
        {
          RawPtr<Node> current = refChild;
          while (current = NodeTraversal::Previous(*current))
          {
            if (current->IsElementNode())
            {
              return Exception {ExceptionCode::HierarchyRequestError};
            }
          }
        }
        else if (hasElementChild)
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }
      }
    }

    return {};
  }

  /// @see https://dom.spec.whatwg.org/#concept-node-pre-insert
  ExceptionOr<Node &> ContainerNode::PreInsert(Node &node, RawPtr<Node> child) noexcept
  {
    if (auto result = EnsurePreInsertValidity(node, child); result.HasException())
    {
      return {result.ReleaseException()};
    }

    if (&node == child)
    {
      child = node.NextSibling();
    }

    auto result = Insert(node, child);
    if (result.HasException())
    {
      return {result.ReleaseException()};
    }

    return node;
  }

  /// @see https://dom.spec.whatwg.org/#concept-node-insert
  ExceptionOr<void> ContainerNode::Insert(Node &node, RawPtr<Node> refChild, bool suppressObservers) noexcept
  {
    auto &parent = *this;

    NodeSmallList targets;
    if (node.IsDocumentFragmentNode())
    {
      CollectChildNodes(node, targets);
    }
    else
    {
      targets.push_back(ShareRef<Node>(node));
    }

    if (targets.empty())
    {
      return {};
    }

    if (node.IsDocumentFragmentNode())
    {
      // TODO(IMPL):
      // remove children from the document fragment with suppressObservers = true
      // Queue mutation record for node with << >>, nodes, null, and null.
    }

    if (refChild != nullptr)
    {
      // TODO(IMPL):
      // For each live range whose start node is parent and start offset is greater than child’s index:
      // increase its start offset by count. For each live range whose end node is parent and end offset is
      // greater than child’s index: increase its end offset by count.
    }

    auto previousSibling = ShareRefPtr<Node>(refChild ? refChild->PreviousSibling() : parent.LastChild());
    for (auto &target : targets)
    {
      if (auto result = parent.OwnerDocument()->AdoptNode(*target); result.HasException())
      {
        return {result.ReleaseException()};
      }

      if (auto result = target->Remove(); result.HasException())
      {
        return {result.ReleaseException()};
      }

      target->SetParentNode(this);

      if (refChild)
      {
        if (refChild->PreviousSibling())
        {
          refChild->PreviousSibling()->SetNextSibling(target.get());
          target->SetPreviousSibling(refChild->PreviousSibling());

          refChild->SetPreviousSibling(target.get());
          target->SetNextSibling(refChild);
        }
        else
        {
          assert(_firstChild == refChild);
          _firstChild = target.get();

          refChild->SetPreviousSibling(target.get());
          target->SetNextSibling(refChild);
        }
      }
      else
      {
        if (auto lastChild = ShareRefPtr(LastChild()))
        {
          target->SetPreviousSibling(lastChild.get());
          lastChild->SetNextSibling(target.get());
        }
        else
        {
          _firstChild = target.get();
        }

        _lastChild = target.get();
      }
    }

    return {};
  }
}
