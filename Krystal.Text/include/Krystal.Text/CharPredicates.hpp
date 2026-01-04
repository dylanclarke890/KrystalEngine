#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <utility>

namespace Krys
{
  /// @brief Internal comparison structure.
  struct EqualToCharFn
  {
    /// @brief Compares two objects using the `==` operator.
    /// @param left The left object.
    /// @param right The right object.
    /// @return If the two types are not character types (`uchar`, `schar`, or `char`) or they are
    /// the same type, simply performs the usual `==` comparison. Otherwise, casts either the left or the
    /// right value to be of the same signedness of the left or right non-`char` character type.
    template <typename TLeft, typename TRight>
    constexpr auto operator()(TLeft &&left, TRight &&right) const noexcept
    {
      using TULeft = remove_cvref_t<TLeft>;
      using TURight = remove_cvref_t<TRight>;
      if constexpr (SameType<TULeft, TURight>)
      {
        return std::forward<TLeft>(left) == std::forward<TRight>(right);
      }
      else if constexpr (SameType<TULeft, char> || SameType<TURight, char>)
      {
        if constexpr (SameType<TULeft, uchar> || SameType<TURight, uchar>)
        {
          return static_cast<uchar>(left) == static_cast<uchar>(right);
        }
        else if constexpr (SameType<TULeft, schar> || SameType<TURight, schar>)
        {
          return static_cast<schar>(left) == static_cast<schar>(right);
        }
        else
        {
          return std::forward<TLeft>(left) == std::forward<TRight>(right);
        }
      }
      else
      {
        return std::forward<TLeft>(left) == std::forward<TRight>(right);
      }
    }
  };

  /// @brief A heterogenous binary equals to comparison predicate each other and makes sure their signedness
  /// does not ruin the comparison. This object can be passed as a parameter, even to templated functions.
  constexpr inline const EqualToCharFn equal_to_char = {};

  /// @brief Internal comparison structure.
  struct NotEqualToCharFn
  {
    /// @brief Compares two objects using the `!=` operator.
    /// @param left The left object.
    /// @param right The right object.
    /// @return If the two types are not character types (`uchar`, `schar`, or `char`) or they are
    /// the same type, simply performs the usual `!=` comparison. Otherwise, casts either the left or the
    /// right value to be of the same signedness of the left or right non-`char` character type.
    template <typename TLeft, typename TRight>
    constexpr auto operator()(TLeft &&left, TRight &&right) const noexcept
    {
      using TULeft = remove_cvref_t<TLeft>;
      using TURight = remove_cvref_t<TRight>;
      if constexpr (SameType<TULeft, TURight>)
      {
        return std::forward<TLeft>(left) == std::forward<TRight>(right);
      }
      else if constexpr (SameType<TULeft, char> || SameType<TURight, char>)
      {
        if constexpr (SameType<TULeft, uchar> || SameType<TURight, uchar>)
        {
          return static_cast<uchar>(left) == static_cast<uchar>(right);
        }
        else if constexpr (SameType<TULeft, schar> || SameType<TURight, schar>)
        {
          return static_cast<schar>(left) == static_cast<schar>(right);
        }
        else
        {
          return std::forward<TLeft>(left) == std::forward<TRight>(right);
        }
      }
      else
      {
        return std::forward<TLeft>(left) == std::forward<TRight>(right);
      }
    }
  };

  /// @brief A heterogenous binary equals to comparison predicate each other and makes sure their signedness
  /// does not ruin the comparison. This object can be passed as a parameter, even to templated functions.
  constexpr inline const NotEqualToCharFn not_equal_to_char = {};
}