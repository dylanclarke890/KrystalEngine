#pragma once

#include "Krystal.HTML/DOM/Internals/TraversalBase.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class Node;

  /// @see https://dom.spec.whatwg.org/#interface-nodeiterator
  class NodeIterator : public TraversalBase, public RefCounted<NodeIterator>
  {
    friend class Document;
    friend class IteratorAlgorithms;

  private:
    Ref<Node> _referenceNode;
    bool _pointerBeforeReferenceNode {false};

    NodeIterator(Node &root, HTML::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept;

  public:
#pragma region NodeIterator - https://dom.spec.whatwg.org/#nodeiterator

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-referencenode
    KRYS_NODISCARD const Node &ReferenceNode() const noexcept
    {
      return *_referenceNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-referencenode
    KRYS_NODISCARD Node &ReferenceNode() noexcept
    {
      return *_referenceNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-pointerbeforereferencenode
    KRYS_NODISCARD bool PointerBeforeReferenceNode() const noexcept
    {
      return _pointerBeforeReferenceNode;
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

  protected:
    void ReferenceNode(Ref<Node> &&refNode) noexcept
    {
      _referenceNode = Krys::Move(refNode);
    }

    void PointerBeforeReferenceNode(bool isBefore) noexcept
    {
      _pointerBeforeReferenceNode = isBefore;
    }
  };
}