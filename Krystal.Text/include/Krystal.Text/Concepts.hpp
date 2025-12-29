#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Rune.hpp"

namespace Krys
{
  template <typename T>
  concept ForEachCodepointCallable = Callable<T, Rune> || Callable<T, Rune, bool>;

  template <typename T>
  concept IsCharacter =
    SameType<T, Rune> || SameType<T, uint8> || SameType<T, byte> || SameType<T, char8> || SameType<T, char>;
}