#include "Krystal.Booey/DOM/Traversal/NodeIterator.hpp"
#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo
{
  NodeIterator::NodeIterator(Node &root, boo::WhatToShow whatToShow, RefPtr<NodeFilter> filter) noexcept
      : dom::Traverser(root, whatToShow, krys::move(filter)), _reference {ShareRef(root), true}
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
    return dom::IteratorAlgorithms::Traverse(*this, dom::TraversalNextOrPrev::Next);
  }

  ExceptionOr<RefPtr<Node>> NodeIterator::PreviousNode() noexcept
  {
    return dom::IteratorAlgorithms::Traverse(*this, dom::TraversalNextOrPrev::Previous);
  }
}
