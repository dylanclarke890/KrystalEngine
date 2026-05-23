#pragma once

#include "Krystal.HTML/Iterator/_detail/IteratorBase.hpp"
#include "Krystal.HTML/Iterator/FilterResult.hpp"
#include "Krystal.HTML/Iterator/NodeFilter.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class Node;

  class NodeIterator : public IteratorBase, public RefCounted<NodeIterator>
  {
    friend class Document;
    friend class IteratorAlgorithms;

  private:
    Ref<Node> _referenceNode;
    bool _pointerBeforeReferenceNode {false};

    NodeIterator(Node &root, HTML::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept;

  public:
    KRYS_NODISCARD const Node &ReferenceNode() const noexcept
    {
      return *_referenceNode;
    }

    KRYS_NODISCARD Node &ReferenceNode() noexcept
    {
      return *_referenceNode;
    }

    KRYS_NODISCARD bool PointerBeforeReferenceNode() const noexcept
    {
      return _pointerBeforeReferenceNode;
    }

    KRYS_NODISCARD ExceptionOr<RefPtr<Node>> NextNode() noexcept;
    KRYS_NODISCARD ExceptionOr<RefPtr<Node>> PreviousNode() noexcept;

    void Detach() const noexcept
    {
      // Does nothing as per the spec.
    }

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