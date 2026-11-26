#include "Krystal.Web/Dom/Traversal/NodeIterator.hpp"
#include "Krystal.Web/Dom/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"

namespace krys::web
{
  NodeIterator::NodeIterator(Node &root, web::WhatToShow whatToShow, RefPtr<NodeFilter> filter) noexcept
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
