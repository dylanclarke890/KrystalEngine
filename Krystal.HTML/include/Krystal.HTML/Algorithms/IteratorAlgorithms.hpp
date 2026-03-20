#pragma once

#include "Krystal.HTML/Iterator/FilterResult.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class IteratorBase;
  class Node;
  class NodeIterator;
  class TreeWalker;

  enum class TraversalType
  {
    Next,
    Previous
  };

  class IteratorAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-node-filter
    KRYS_NODISCARD static ExceptionOr<FilterResult> FilterNode(IteratorBase &traverser,
                                                               const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#nodeiterator-pre-removing-steps
    KRYS_NODISCARD static void PreRemove(NodeIterator &nodeIterator, const Node &toBeRemovedNode) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-nodeiterator-traverse
    KRYS_NODISCARD static ExceptionOr<RefPtr<Node>> Traverse(NodeIterator &iterator,
                                                             TraversalType type) noexcept;
  };
}