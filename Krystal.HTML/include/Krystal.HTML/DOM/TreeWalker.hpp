#pragma once

#include "Krystal.HTML/DOM/Callbacks/NodeFilter.hpp"
#include "Krystal.HTML/DOM/Enums/FilterResult.hpp"
#include "Krystal.HTML/DOM/Internals/TraversalBase.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#interface-treewalker
  class TreeWalker : public TraversalBase, public RefCounted<TreeWalker>
  {
    friend class Document;

  private:
    Ref<Node> _currentNode;

  protected:
    TreeWalker(Node &root, HTML::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept;

  public:
#pragma region TreeWalker - https://dom.spec.whatwg.org/#treewalker

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-currentnode
    KRYS_NODISCARD const Node &CurrentNode() const noexcept
    {
      return *_currentNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-currentnode
    KRYS_NODISCARD Node &CurrentNode() noexcept
    {
      return *_currentNode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-currentnode
    void CurrentNode(Node &currentNode) noexcept
    {
      _currentNode = ShareRef(currentNode);
    }

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-parentnode
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> ParentNode() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-firstchild
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> FirstChild() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-lastchild
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> LastChild() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-previoussibling
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> PreviousSibling() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-nextsibling
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> NextSibling() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-previousnode
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> PreviousNode() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-nextnode
    KRYS_NODISCARD ExceptionOr<RawPtr<Node>> NextNode() noexcept;

#pragma endregion
  };
}