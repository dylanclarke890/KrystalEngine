#pragma once

#include "Krystal.Booey/DOM/Traversal/Traverser.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Document;
  class Node;
}

namespace krys::boo::dom
{
  /// @brief A node, and whether the pointer is before or after it.
  /// @see https://dom.spec.whatwg.org/#node-pointer
  struct NodePointer
  {
    /// @see https://dom.spec.whatwg.org/#node-pointer-node
    Ref<boo::Node> Node;
    /// @see https://dom.spec.whatwg.org/#node-pointer-pointer-before
    bool PointerBefore {false};
  };
}

namespace krys::boo
{
  /// @brief Iterates over the inclusive descendants of its root (its iterator collection) in tree order.
  /// @see https://dom.spec.whatwg.org/#interface-nodeiterator
  /// @see https://dom.spec.whatwg.org/#iterator-collection
  class NodeIterator : public dom::Traverser, public RefCounted<NodeIterator>
  {
    friend class Document;
    friend class dom::IteratorAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#nodeiterator-reference
    dom::NodePointer _reference;
    /// @see https://dom.spec.whatwg.org/#nodeiterator-candidate-reference
    Maybe<dom::NodePointer> _candidateReference;

    NodeIterator(Node &root, boo::WhatToShow whatToShow, RefPtr<NodeFilter> filter) noexcept;

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

#pragma endregion
  };
}
