#pragma once

#include "Krystal.Lib/Ranges/Reconstruct.hpp"

namespace Krys::Impl
{
  template <bool IsMutable, typename TExpected, typename TInput>
  constexpr bool IsUpdateInputNoexcept() noexcept
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUExpected = remove_cvref_t<TExpected>;
    if constexpr (SameType<TUInput, TUExpected>)
    {
      return true;
    }
    else
    {
      if constexpr (IsMutable)
      {
        return Krys::Ranges::NoThrowRangeReconstructible<TUExpected, TInput>;
      }
      else
      {
        return Krys::Ranges::NoThrowRangeConstReconstructible<TUExpected, TInput>;
      }
    }
  }

  template <typename TExpected, typename TInput>
  constexpr decltype(auto)
    UpdateInput(TInput &&input) noexcept(IsUpdateInputNoexcept<false, TExpected, TInput>())
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUExpected = remove_cvref_t<TExpected>;
    if constexpr (SameType<TUInput, TUExpected>)
    {
      return std::forward<TInput>(input);
    }
    else
    {
      return Krys::Ranges::reconstruct(std::in_place_type<TUExpected>, std::forward<TInput>(input));
    }
  }

  template <typename TExpected, typename TInput>
  constexpr decltype(auto)
    UpdateConstInput(TInput &&input) noexcept(IsUpdateInputNoexcept<true, TExpected, TInput>())
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUExpected = remove_cvref_t<TExpected>;
    if constexpr (SameType<TUInput, TUExpected>)
    {
      return std::forward<TInput>(input);
    }
    else
    {
      return Krys::Ranges::const_reconstruct(std::in_place_type<TUExpected>, std::forward<TInput>(input));
    }
  }
}
