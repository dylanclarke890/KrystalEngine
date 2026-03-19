#pragma once

#include "Krystal.HTML/Iterator/WhatToShow.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class IteratorAlgorithms;

  class IteratorBase
  {
    friend class IteratorAlgorithms;

  protected:
    Ref<Node> _root;
    RefPtr<NodeFilter> _filter;
    WhatToShow _whatToShow {WhatToShow::SHOW_ALL};
    bool _isActive {false};

  public:
    KRYS_NODISCARD Node &Root() noexcept
    {
      return *_root;
    }

    KRYS_NODISCARD const Node &Root() const noexcept
    {
      return *_root;
    }

    KRYS_NODISCARD WhatToShow WhatToShow() const noexcept
    {
      return _whatToShow;
    }

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