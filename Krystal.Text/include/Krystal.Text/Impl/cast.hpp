#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include <utility>

namespace Krys::Impl
{
  enum class MatchAlignment
  {
    no,
    yes
  };

  template <typename TTo, MatchAlignment RequireAligned = MatchAlignment::no, typename TFrom>
  constexpr decltype(auto) StaticCastIfLossless(TFrom &&from)
  {
    if constexpr ((sizeof(remove_cvref_t<TTo>) == sizeof(remove_cvref_t<TFrom>))
                  && ((RequireAligned == MatchAlignment::no)
                      || (alignof(remove_cvref_t<TTo>) == alignof(remove_cvref_t<TFrom>))))
    {
      // explicitly cast, since we know it's of the same size/alignment (e.g., uchar -> std::byte should
      // work, but it requires a cast!)
      return static_cast<TTo>(from);
    }
    else
    {
      // let it warn/error for weird conversions (e.g., short -> char8_t should give a narrowing conversion
      // warning)
      return std::forward<TFrom>(from);
    }
  }
}
