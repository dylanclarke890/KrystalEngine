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
      // Let next be toBeRemovedNode’s first following node that is an inclusive descendant of nodeIterator’s
      // root and is not an inclusive descendant of toBeRemovedNode, if there is such a node; otherwise null.
      RawPtr<Node> next = nullptr;
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
}
