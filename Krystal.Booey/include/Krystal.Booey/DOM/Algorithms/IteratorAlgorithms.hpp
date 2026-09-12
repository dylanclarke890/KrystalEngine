#pragma once

#include "Krystal.Booey/DOM/Enums/FilterResult.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class TraversalBase;
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
    KRYS_NODISCARD static ExceptionOr<FilterResult> FilterNode(TraversalBase &traverser,
                                                               const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#nodeiterator-pre-removing-steps
    KRYS_NODISCARD static void PreRemove(NodeIterator &nodeIterator, const Node &toBeRemovedNode) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-nodeiterator-traverse
    KRYS_NODISCARD static ExceptionOr<RefPtr<Node>> Traverse(NodeIterator &iterator,
                                                             TraversalNextOrPrev type) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-traverse-children
    KRYS_NODISCARD static ExceptionOr<Node *> TraverseChildren(TreeWalker &walker,
                                                               TraverseFirstOrLast type) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-traverse-siblings
    KRYS_NODISCARD static ExceptionOr<Node *> TraverseSiblings(TreeWalker &walker,
                                                               TraversalNextOrPrev type) noexcept;
  };
}