#include "Krystal.HTML/Tree/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Tree/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"

namespace Krys::HTML
{
  ExceptionOr<void> TreeMutationAlgorithms::EnsurePreInsertValidity(Node &node, ContainerNode &parent,
                                                                    RawPtr<Node> refChild) noexcept
  {
    if (!parent.IsDocumentNode() && !parent.IsDocumentFragmentNode() && !parent.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (TreeQueries::IsHostIncludingAncestorOf(node, parent))
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

          if (TreeQueries::IsDocTypeOrDocTypeFollows(refChild))
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

        if (TreeQueries::IsDocTypeOrDocTypeFollows(refChild))
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
          while (current = TreeTraversal::Previous(*current))
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

  ExceptionOr<void> TreeMutationAlgorithms::PreInsert(Node &node, ContainerNode &parent,
                                                      RawPtr<Node> refChild) noexcept
  {
    if (auto result = EnsurePreInsertValidity(node, parent, refChild); result.HasException())
    {
      return {result.ReleaseException()};
    }

    return {};
  }

  ExceptionOr<void> TreeMutationAlgorithms::Insert(Node &node, ContainerNode &parent, RawPtr<Node> refChild,
                                                   SuppressObservers suppressObservers) noexcept
  {
    SmallNodeList targets;
    if (node.IsDocumentFragmentNode())
    {
      TreeQueries::CollectChildNodes(static_cast<ContainerNode &>(node), targets);
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

    // This step is part of the PreInsert algorithm but it'd be annoying to take a pointer to a pointer to be
    // able to update this properly
    if (&node == refChild)
    {
      refChild = node.NextSibling();
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

      target->SetParentNode(&parent);

      if (refChild)
      {
        if (auto previousSibling = ShareRefPtr(refChild->PreviousSibling()))
        {
          previousSibling->SetNextSibling(target.get());
          target->SetPreviousSibling(previousSibling.get());

          refChild->SetPreviousSibling(target.get());
          target->SetNextSibling(refChild);
        }
        else
        {
          assert(parent.FirstChild() == refChild);
          parent.SetFirstChild(target.get());

          refChild->SetPreviousSibling(target.get());
          target->SetNextSibling(refChild);
        }
      }
      else
      {
        if (auto lastChild = ShareRefPtr(parent.LastChild()))
        {
          target->SetPreviousSibling(lastChild.get());
          lastChild->SetNextSibling(target.get());
        }
        else
        {
          parent.SetFirstChild(target.get());
        }

        parent.SetLastChild(target.get());
      }

      TreeMutationDispatcher::NodeInserted(*target, parent);

      target->SetTreeScopeRecursively(*parent.OwnerDocument());
    }

    return {};
  }

  ExceptionOr<void> TreeMutationAlgorithms::PreRemove(Node &node, ContainerNode &parent) noexcept
  {
    if (node.ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    return {};
  }

  ExceptionOr<void> TreeMutationAlgorithms::Remove(Node &node, ContainerNode &parent,
                                                   SuppressObservers suppressObservers) noexcept
  {
    if (auto previousSibling = ShareRefPtr(node.PreviousSibling()))
    {
      previousSibling->SetNextSibling(node.NextSibling());
    }
    else
    {
      assert(parent.FirstChild() == &node);
      parent.SetFirstChild(node.NextSibling());
    }

    if (auto nextSibling = ShareRefPtr(node.NextSibling()))
    {
      nextSibling->SetPreviousSibling(node.PreviousSibling());
    }
    else
    {
      assert(parent.LastChild() == &node);
      parent.SetLastChild(node.PreviousSibling());
    }

    node.SetParentNode(nullptr);
    node.SetPreviousSibling(nullptr);
    node.SetNextSibling(nullptr);

    assert(parent.FirstChild() != &node);
    assert(parent.LastChild() != &node);
    assert(!node.PreviousSibling());
    assert(!node.NextSibling());

    TreeMutationDispatcher::NodeRemoved(node, parent);

    return {};
  }
}
