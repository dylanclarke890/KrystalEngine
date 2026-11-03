#pragma once

#include "Krystal.Booey/DOM/Traversal/NodeFilter.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Node;

  /// @brief A NodeIterator or TreeWalker: a root, the node types it shows and a filter.
  /// @see https://dom.spec.whatwg.org/#traversal
  class TraversalBase
  {
    friend class IteratorAlgorithms;

  protected:
    Ref<Node> _root;
    RefPtr<NodeFilter> _filter;
    WhatToShow _whatToShow {WhatToShow::SHOW_ALL};
    /// @see https://dom.spec.whatwg.org/#concept-traversal-active
    bool _isActive {false};

  protected:
    TraversalBase(Node &root, WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept
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

  protected:
    /// @see https://dom.spec.whatwg.org/#concept-traversal-active
    KRYS_NODISCARD bool IsActive() const noexcept
    {
      return _isActive;
    }

    void IsActive(bool active) noexcept
    {
      _isActive = active;
    }
  };
}