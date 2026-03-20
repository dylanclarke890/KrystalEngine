#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Iterator/NodeIterator.hpp"
#include "Krystal.HTML/Iterator/TreeWalker.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  ExceptionOr<FilterResult> IteratorAlgorithms::FilterNode(IteratorBase &traverser, const Node &node) noexcept
  {
    if (traverser.IsActive())
    {
      return Exception {ExceptionCode::InvalidStateError};
    }

    auto n = 1u << (static_cast<underlying_t<NodeType>>(node.GetNodeType()) - 1u);
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
    auto inclusiveAncestor = !TreeQueries::IsInclusiveAncestor(toBeRemovedNode, nodeIterator.ReferenceNode());
    if (inclusiveAncestor || &toBeRemovedNode == &nodeIterator.Root())
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

  ExceptionOr<RefPtr<Node>> IteratorAlgorithms::Traverse(NodeIterator &iterator, TraversalType type) noexcept
  {
    RawPtr<Node> node = &iterator.ReferenceNode();
    bool beforeNode = iterator.PointerBeforeReferenceNode();

    while (true)
    {
      if (type == TraversalType::Next)
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
        beforeNode = false;
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
}
