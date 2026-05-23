#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys::Text
{
  /// @brief Retrieves the code point type for the given encoding type.
  template <typename T>
  class CodePoint
  {
  public:
    /// @brief The code point type for the given type. If it does not exist, UnicodeCodePoint is assumed.
    using type = typename remove_cvref_t<T>::code_point;
  };

  /// @brief Get the underlying type of a CodePoint.
  template <typename T>
  using code_point_t = typename CodePoint<T>::type;

  /// @brief Gets the max number of code points that can be produced by an encoding during a decode operation.
  template <typename T>
  constexpr inline static std::size_t MaxCodePoints = remove_cvref_t<T>::MaxCodePoints;
}