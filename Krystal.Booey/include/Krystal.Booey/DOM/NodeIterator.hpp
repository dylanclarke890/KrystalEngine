#pragma once

#include "Krystal.Booey/DOM/Internals/TraversalBase.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Node;

  /// @brief A node, and whether the pointer is before or after it.
  /// @see https://dom.spec.whatwg.org/#node-pointer
  struct NodePointer
  {
    /// @see https://dom.spec.whatwg.org/#node-pointer-node
    Ref<dom::Node> Node;
    /// @see https://dom.spec.whatwg.org/#node-pointer-pointer-before
    bool PointerBefore {false};
  };

  /// @brief Iterates over the inclusive descendants of its root (its iterator collection) in tree order.
  /// @see https://dom.spec.whatwg.org/#interface-nodeiterator
  /// @see https://dom.spec.whatwg.org/#iterator-collection
  class NodeIterator : public TraversalBase, public RefCounted<NodeIterator>
  {
    friend class Document;
    friend class IteratorAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#nodeiterator-reference
    NodePointer _reference;
    /// @see https://dom.spec.whatwg.org/#nodeiterator-candidate-reference
    Maybe<NodePointer> _candidateReference;

    NodeIterator(Node &root, dom::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept;

  public:
    ~NodeIterator() noexcept;

#pragma region NodeIterator - https://dom.spec.whatwg.org/#nodeiterator

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-referencenode
    KRYS_NODISCARD const Node &ReferenceNode() const noexcept
    {
      return *_reference.Node;
    }

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-referencenode
    KRYS_NODISCARD Node &ReferenceNode() noexcept
    {
      return *_reference.Node;
    }

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-pointerbeforereferencenode
    KRYS_NODISCARD bool PointerBeforeReferenceNode() const noexcept
    {
      return _reference.PointerBefore;
    }

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-nextnode
    KRYS_NODISCARD ExceptionOr<RefPtr<Node>> NextNode() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-previousnode
    KRYS_NODISCARD ExceptionOr<RefPtr<Node>> PreviousNode() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-detach
    void Detach() const noexcept
    {
      // Does nothing as per the spec.
    }

#pragma endregion
  };
}