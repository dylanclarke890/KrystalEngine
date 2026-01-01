#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  class UnicodeScalarValue;
  class UnicodeCodePoint;

  template <typename T>
  concept IsUnicodeScalarValue = SameType<remove_cvref_t<T>, UnicodeScalarValue>;

  template <typename T>
  concept IsUnicodeCodePoint =
    SameType<remove_cvref_t<T>, char32> || SameType<remove_cvref_t<T>, UnicodeCodePoint>
    || IsUnicodeScalarValue<remove_cvref_t<T>>;
}