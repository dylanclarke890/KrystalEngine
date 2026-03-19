#pragma once

#include "Krystal.HTML/Iterator/FilterResult.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"

namespace Krys::HTML
{
  class Node;
  class IteratorBase;
  class TreeWalker;

  class IteratorAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-node-filter
    KRYS_NODISCARD static ExceptionOr<FilterResult> FilterNode(IteratorBase &traverser,
                                                               const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#nodeiterator-pre-removing-steps
    KRYS_NODISCARD static void PreRemove(NodeIterator &nodeIterator, const Node &toBeRemovedNode) noexcept;
  };
}