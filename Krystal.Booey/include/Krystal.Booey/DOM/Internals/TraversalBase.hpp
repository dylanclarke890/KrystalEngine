#pragma once

#include "Krystal.Booey/DOM/Callbacks/NodeFilter.hpp"
#include "Krystal.Booey/DOM/Enums/WhatToShow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Node;

  /// @see https://dom.spec.whatwg.org/#traversal
  class TraversalBase
  {
    friend class IteratorAlgorithms;

  protected:
    Ref<Node> _root;
    RefPtr<NodeFilter> _filter;
    WhatToShow _whatToShow {WhatToShow::SHOW_ALL};
    bool _isActive {false};

  protected:
    TraversalBase(Node &root, WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept
        : _root(ShareRef(root)), _filter(krys::move(filter)), _whatToShow(whatToShow)
    {
    }

  public:
    /// @see https://dom.spec.whatwg.org/#concept-traversal-root
    KRYS_NODISCARD Node &Root() noexcept
    {
      return *_root;
    }

    /// @see https://dom.spec.whatwg.org/#concept-traversal-root
    KRYS_NODISCARD const Node &Root() const noexcept
    {
      return *_root;
    }

    /// @see https://dom.spec.whatwg.org/#concept-traversal-whattoshow
    KRYS_NODISCARD WhatToShow WhatToShow() const noexcept
    {
      return _whatToShow;
    }

    /// @see https://dom.spec.whatwg.org/#concept-traversal-filter
    KRYS_NODISCARD NodeFilter *Filter() const noexcept
    {
      return _filter.get();
    }

  protected:
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