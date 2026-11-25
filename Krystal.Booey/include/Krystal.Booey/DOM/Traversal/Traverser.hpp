#pragma once

#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Traversal/NodeFilter.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class Node;
  class NodeIterator;
  class TreeWalker;
}

namespace krys::boo::dom
{
  /// @brief What a NodeIterator and a TreeWalker share: a root, the node types it shows, a filter, and
  /// whether it's filtering a node. The specification's filter algorithm takes either as its traverser.
  /// @see https://dom.spec.whatwg.org/#traversal
  /// @see https://dom.spec.whatwg.org/#concept-node-filter
  class Traverser
  {
    friend class boo::NodeIterator;
    friend class boo::TreeWalker;
    friend class IteratorAlgorithms;

  private:
    Ref<Node> _root;
    RefPtr<NodeFilter> _filter;
    WhatToShow _whatToShow {ShowAll};
    /// @see https://dom.spec.whatwg.org/#concept-traversal-active
    bool _isActive {false};

    Traverser(Node &root, WhatToShow whatToShow, RefPtr<NodeFilter> filter) noexcept
        : _root(ShareRef(root)), _filter(krys::move(filter)), _whatToShow(whatToShow)
    {
    }

  public:
    /// @see https://dom.spec.whatwg.org/#concept-traversal-root
    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-root
    /// @see https://dom.spec.whatwg.org/#dom-treewalker-root
    KRYS_NODISCARD Node &Root() noexcept
    {
      return *_root;
    }

    /// @see https://dom.spec.whatwg.org/#concept-traversal-root
    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-root
    /// @see https://dom.spec.whatwg.org/#dom-treewalker-root
    KRYS_NODISCARD const Node &Root() const noexcept
    {
      return *_root;
    }

    /// @see https://dom.spec.whatwg.org/#concept-traversal-whattoshow
    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-whattoshow
    /// @see https://dom.spec.whatwg.org/#dom-treewalker-whattoshow
    KRYS_NODISCARD WhatToShow WhatToShow() const noexcept
    {
      return _whatToShow;
    }

    /// @see https://dom.spec.whatwg.org/#concept-traversal-filter
    /// @see https://dom.spec.whatwg.org/#dom-nodeiterator-filter
    /// @see https://dom.spec.whatwg.org/#dom-treewalker-filter
    KRYS_NODISCARD NodeFilter *Filter() const noexcept
    {
      return _filter.get();
    }

  private:
    /// @see https://dom.spec.whatwg.org/#concept-traversal-active
    KRYS_NODISCARD bool IsActive() const noexcept
    {
      return _isActive;
    }

    void SetActive(bool active) noexcept
    {
      _isActive = active;
    }
  };
}
