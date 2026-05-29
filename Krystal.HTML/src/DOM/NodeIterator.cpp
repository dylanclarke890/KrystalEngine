#include "Krystal.HTML/DOM/NodeIterator.hpp"
#include "Krystal.HTML/DOM/Algorithms/IteratorAlgorithms.hpp"

namespace Krys::HTML
{
  NodeIterator::NodeIterator(Node &root, HTML::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept
      : IteratorBase(root, whatToShow, Krys::Move(filter)), _referenceNode(ShareRef(root)),
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