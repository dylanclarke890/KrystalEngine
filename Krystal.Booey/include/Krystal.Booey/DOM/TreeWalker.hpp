#pragma once

#include "Krystal.Booey/DOM/Callbacks/NodeFilter.hpp"
#include "Krystal.Booey/DOM/Enums/FilterResult.hpp"
#include "Krystal.Booey/DOM/Internals/TraversalBase.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#interface-treewalker
  class TreeWalker : public TraversalBase, public RefCounted<TreeWalker>
  {
    friend class Document;

  private:
    Ref<Node> _currentNode;

  protected:
    TreeWalker(Node &root, dom::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept;

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
    KRYS_NODISCARD ExceptionOr<Node *> ParentNode() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-firstchild
    KRYS_NODISCARD ExceptionOr<Node *> FirstChild() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-lastchild
    KRYS_NODISCARD ExceptionOr<Node *> LastChild() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-previoussibling
    KRYS_NODISCARD ExceptionOr<Node *> PreviousSibling() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-nextsibling
    KRYS_NODISCARD ExceptionOr<Node *> NextSibling() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-previousnode
    KRYS_NODISCARD ExceptionOr<Node *> PreviousNode() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-treewalker-nextnode
    KRYS_NODISCARD ExceptionOr<Node *> NextNode() noexcept;

#pragma endregion
  };
}