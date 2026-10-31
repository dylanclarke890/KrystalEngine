#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"

namespace krys::boo::dom
{
  NodeIterator::NodeIterator(Node &root, dom::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept
      : TraversalBase(root, whatToShow, krys::move(filter)), _referenceNode(ShareRef(root)),
        _pointerBeforeReferenceNode(true)
  {
    // The document runs the NodeIterator pre-remove steps for the iterators whose root it is the node
    // document of.
    root.NodeDocument().NodeIterators().push_back(this);
  }

  NodeIterator::~NodeIterator() noexcept
  {
    std::erase(Root().NodeDocument().NodeIterators(), this);
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