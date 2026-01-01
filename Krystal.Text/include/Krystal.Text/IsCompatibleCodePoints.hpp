#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief Checks whether the two code point types are compatible with one another.
  /// @tparam TLeftCodePoint A code point type.
  /// @tparam TRightCodePoint Another code point type.
  /// @remarks The `value` boolean is true if the given code point types are:
  /// - both the same type
  /// - both considered unicode code points by IsUnicodeCodePoint
  template <typename TLeftCodePoint, typename TRightCodePoint>
  concept IsCompatibleCodePoints =
    SameType<TLeftCodePoint, TRightCodePoint>
    || (IsUnicodeCodePoint<TLeftCodePoint> && IsUnicodeCodePoint<TRightCodePoint>);
}
