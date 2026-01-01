#pragma once

namespace Krys::Ranges
{
  /// @brief A sentinel that cannot compare equal to any other iterator and thus results in infinitely long
  /// ranges.
  class UnreachableSentinel
  {
    /// @brief Equality comparison. Always false.
    template <typename TLeft>
    friend constexpr bool operator==(const TLeft &, const UnreachableSentinel &)
    {
      return false;
    }

    /// @brief Equality comparison. Always false.
    template <typename TRight>
    friend constexpr bool operator==(const UnreachableSentinel &, const TRight &)
    {
      return false;
    }

    /// @brief Inequality comparison. Always true.
    template <typename TLeft>
    friend constexpr bool operator!=(const TLeft &, const UnreachableSentinel &)
    {
      return true;
    }

    /// @brief Inequality comparison. Always true.
    template <typename TRight>
    friend constexpr bool operator!=(const UnreachableSentinel &, const TRight &)
    {
      return true;
    }
  };

  /// @brief An available and usable ztd::ranges::UnreachableSentinel for ease of use.
  inline constexpr UnreachableSentinel unreachable_sentinel = {};
}
