#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"

namespace krys::boo::dom
{
  NodeIterator::NodeIterator(Node &root, dom::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept
      : TraversalBase(root, whatToShow, krys::move(filter)), _referenceNode(ShareRef(root)),
        _pointerBeforeReferenceNode(true)
  {
  }

  ExceptionOr<RefPtr<Node>> NodeIterator::NextNode() noexcept
  {
    return IteratorAlgorithms::Traverse(*this, TraversalNextOrPrev::Next);
  }

  ExceptionOr<RefPtr<Node>> NodeIterator::PreviousNode() noexcept
  {
    return IteratorAlgorithms::Traverse(*this, TraversalNextOrPrev::Previous);
  }
}