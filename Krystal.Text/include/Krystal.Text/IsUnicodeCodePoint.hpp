#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include "Krystal.Text/UnicodeScalarValue.hpp"

namespace Krys::Text
{
  template <typename T>
  concept IsUnicodeScalarValue = SameType<remove_cvref_t<T>, UnicodeScalarValue>;

  template <typename T>
  concept IsUnicodeCodePoint =
    SameType<remove_cvref_t<T>, char32> || SameType<remove_cvref_t<T>, UnicodeCodePoint>
    || IsUnicodeScalarValue<remove_cvref_t<T>>;
}