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
  class TreeWalker : public RefCounted<TreeWalker>, public IteratorBase
  {
    friend class Document;

  private:
    Ref<Node> _currentNode;

  protected:
    TreeWalker(Node &root, HTML::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept;

  public:
    KRYS_NODISCARD const Node &CurrentNode() const noexcept
    {
      return *_currentNode;
    }

    KRYS_NODISCARD Node &CurrentNode() noexcept
    {
      return *_currentNode;
    }

    void CurrentNode(Node &currentNode) noexcept
    {
      _currentNode = ShareRef(currentNode);
    }

    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> ParentNode() noexcept;
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> FirstChild() noexcept;
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> LastChild() noexcept;
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> PreviousSibling() noexcept;
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> NextSibling() noexcept;
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> PreviousNode() noexcept;
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> NextNode() noexcept;
  };
}