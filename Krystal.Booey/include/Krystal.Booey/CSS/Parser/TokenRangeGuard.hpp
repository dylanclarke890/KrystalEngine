#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  struct KRYS_NODISCARD TokenRangeGuard
  {
  private:
    bool _committed {false};
    TokenRange &_range;
    TokenRange _savedRange;

  public:
    TokenRangeGuard(TokenRange &tokens) noexcept : _range {tokens}, _savedRange {tokens}
    {
    }

    ~TokenRangeGuard() noexcept
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