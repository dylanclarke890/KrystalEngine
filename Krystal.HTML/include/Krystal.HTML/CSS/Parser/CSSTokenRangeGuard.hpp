#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
{
  struct KRYS_NODISCARD CSSTokenRangeGuard
  {
  private:
    bool _committed {false};
    CSSTokenRange &_range;
    CSSTokenRange _savedRange;

  public:
    CSSTokenRangeGuard(CSSTokenRange &tokens) noexcept : _range {tokens}, _savedRange {tokens}
    {
    }

    ~CSSTokenRangeGuard() noexcept
    {
      if (_committed)
      {
        return;
      }

      _range = _savedRange;
    }

    void Commit() noexcept
    {
      _committed = true;
    }
  };
}