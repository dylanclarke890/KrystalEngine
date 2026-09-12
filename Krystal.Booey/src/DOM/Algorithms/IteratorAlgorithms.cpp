#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/TreeWalker.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  ExceptionOr<FilterResult> IteratorAlgorithms::FilterNode(TraversalBase &traverser,
                                                           const Node &node) noexcept
  {
    if (traverser.IsActive())
    {
      return Exception {ExceptionCode::InvalidStateError};
    }

    auto n = 1u << (static_cast<underlying_t<NodeType>>(node.NodeType()) - 1u);
    auto whatToShow = static_cast<underlying_t<WhatToShow>>(traverser.WhatToShow());
    if ((whatToShow & n) == 0u)
    {
      return FilterResult::FILTER_SKIP;
    }

    if (!traverser.Filter())
    {
      return FilterResult::FILTER_ACCEPT;
    }

    traverser.IsActive(true);
    auto result = traverser.Filter()->AcceptNode(node);
    traverser.IsActive(false);

    return result;
  }

  void IteratorAlgorithms::PreRemove(NodeIterator &nodeIterator, const Node &toBeRemovedNode) noexcept
  {
    if (!TreeQueries::IsInclusiveAncestor(toBeRemovedNode, nodeIterator.ReferenceNode())
        || &toBeRemovedNode == &nodeIterator.Root())
    {
      return;
    }

    if (nodeIterator.PointerBeforeReferenceNode())
    {
      Node *next = TreeQueries::NextSkippingChildren(toBeRemovedNode);
      if (!next || !TreeQueries::IsInclusiveDescendant(*next, nodeIterator.Root()))
      {
        next = nullptr;
      }

      if (next != nullptr)
      {
        nodeIterator.ReferenceNode(ShareRef(*next));
      }

      nodeIterator.PointerBeforeReferenceNode(false);
    }

    Node *prev = !toBeRemovedNode.PreviousSibling()
                          ? toBeRemovedNode.ParentNode()
                          : TreeQueries::DeepLastChild(*toBeRemovedNode.PreviousSibling());
    assert(prev);
    nodeIterator.ReferenceNode(ShareRef(*prev));
  }

  ExceptionOr<RefPtr<Node>> IteratorAlgorithms::Traverse(NodeIterator &iterator,
                                                         TraversalNextOrPrev type) noexcept
  {
    Node *node = &iterator.ReferenceNode();
    bool beforeNode = iterator.PointerBeforeReferenceNode();

    while (true)
    {
      if (type == TraversalNextOrPrev::Next)
      {
        if (!beforeNode)
        {
          node = TreeQueries::Next(*node, &iterator.Root());
          if (node == nullptr)
          {
            return nullptr;
          }
        }

        beforeNode = false;
      }
      else
      {
        if (beforeNode)
        {
          node = TreeQueries::Previous(*node, &iterator.Root());
          if (node == nullptr)
          {
            return nullptr;
          }
        }
        beforeNode = true;
      }

      auto result = FilterNode(iterator, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        break;
      }
    }

    iterator.ReferenceNode(ShareRef(*node));
    iterator.PointerBeforeReferenceNode(beforeNode);

    return ShareRefPtr(node);
  }

  ExceptionOr<Node *> IteratorAlgorithms::TraverseChildren(TreeWalker &walker,
                                                                 TraverseFirstOrLast type) noexcept
  {
    Node *node = &walker.CurrentNode();
    node = type == TraverseFirstOrLast::First ? node->FirstChild() : node->LastChild();

    while (node != nullptr)
    {
      auto result = FilterNode(walker, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        walker.CurrentNode(*node);
        return node;
      }
      else if (result.Value() == FilterResult::FILTER_SKIP)
      {
        auto child = type == TraverseFirstOrLast::First ? node->FirstChild() : node->LastChild();
        if (child)
        {
          node = child;
          continue;
        }
      }

      while (node != nullptr)
      {
        auto *sibling = type == TraverseFirstOrLast::First ? node->NextSibling() : node->PreviousSibling();
        if (sibling != nullptr)
        {
          node = sibling;
          break;
        }

        auto *parent = node != nullptr ? node->ParentNode() : nullptr;
        if (parent == nullptr || parent == &walker.Root() || parent == &walker.CurrentNode())
        {
          return nullptr;
        }

        node = parent;
      }
    }

    return nullptr;
  }

  ExceptionOr<Node *> IteratorAlgorithms::TraverseSiblings(TreeWalker &walker,
                                                                 TraversalNextOrPrev type) noexcept
  {
    Node *node = &walker.CurrentNode();
    if (node == &walker.Root())
    {
      return nullptr;
    }

    while (true)
    {
      auto sibling = type == TraversalNextOrPrev::Next ? node->NextSibling() : node->PreviousSibling();
      while (sibling != nullptr)
      {
        node = sibling;

        auto result = FilterNode(walker, *node);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        if (result.Value() == FilterResult::FILTER_ACCEPT)
        {
          walker.CurrentNode(*node);
          return node;
        }

        sibling = type == TraversalNextOrPrev::Next ? node->FirstChild() : node->LastChild();

        if (result.Value() == FilterResult::FILTER_REJECT || sibling == nullptr)
        {
          sibling = type == TraversalNextOrPrev::Next ? node->NextSibling() : node->PreviousSibling();
        }
      }

      node = node->ParentNode();
      if (node == nullptr || node == &walker.Root())
      {
        return nullptr;
      }

      auto result = FilterNode(walker, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        return nullptr;
      }
    }
  }
}
