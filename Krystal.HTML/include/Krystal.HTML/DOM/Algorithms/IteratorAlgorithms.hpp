#pragma once

#include "Krystal.HTML/DOM/Enums/FilterResult.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class IteratorBase;
  class Node;
  class NodeIterator;
  class TreeWalker;

  enum class TraversalNextOrPrev
  {
    Next,
    Previous
  };

  enum class TraverseFirstOrLast
  {
    First,
    Last
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
                                                             TraversalNextOrPrev type) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-traverse-children
    KRYS_NODISCARD static ExceptionOr<RawPtr<Node>> TraverseChildren(TreeWalker &walker,
                                                                     TraverseFirstOrLast type) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-traverse-siblings
    KRYS_NODISCARD static ExceptionOr<RawPtr<Node>> TraverseSiblings(TreeWalker &walker,
                                                                     TraversalNextOrPrev type) noexcept;
  };
}