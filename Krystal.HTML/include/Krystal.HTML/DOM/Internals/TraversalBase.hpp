#pragma once

#include "Krystal.HTML/DOM/Callbacks/NodeFilter.hpp"
#include "Krystal.HTML/DOM/Enums/WhatToShow.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
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
        : _root(ShareRef(root)), _filter(Krys::Move(filter)), _whatToShow(whatToShow)
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
    KRYS_NODISCARD RawPtr<NodeFilter> Filter() const noexcept
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