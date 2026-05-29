#include "Krystal.HTML/DOM/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/NodeIterator.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/TreeWalker.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  ExceptionOr<FilterResult> IteratorAlgorithms::FilterNode(IteratorBase &traverser, const Node &node) noexcept
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
      RawPtr<Node> next = TreeTraversal::NextSkippingChildren(toBeRemovedNode);
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

    RawPtr<Node> prev = !toBeRemovedNode.PreviousSibling()
                          ? toBeRemovedNode.ParentNode()
                          : TreeTraversal::DeepLastChild(*toBeRemovedNode.PreviousSibling());
    assert(prev);
    nodeIterator.ReferenceNode(ShareRef(*prev));
  }

  ExceptionOr<RefPtr<Node>> IteratorAlgorithms::Traverse(NodeIterator &iterator,
                                                         TraversalNextOrPrev type) noexcept
  {
    RawPtr<Node> node = &iterator.ReferenceNode();
    bool beforeNode = iterator.PointerBeforeReferenceNode();

    while (true)
    {
      if (type == TraversalNextOrPrev::Next)
      {
        if (!beforeNode)
        {
          node = TreeTraversal::Next(*node, &iterator.Root());
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
          node = TreeTraversal::Previous(*node, &iterator.Root());
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

  ExceptionOr<RawPtr<Node>> IteratorAlgorithms::TraverseChildren(TreeWalker &walker,
                                                                 TraverseFirstOrLast type) noexcept
  {
    RawPtr<Node> node = &walker.CurrentNode();
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

  ExceptionOr<RawPtr<Node>> IteratorAlgorithms::TraverseSiblings(TreeWalker &walker,
                                                                 TraversalNextOrPrev type) noexcept
  {
    RawPtr<Node> node = &walker.CurrentNode();
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
