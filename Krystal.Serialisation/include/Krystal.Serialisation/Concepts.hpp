#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Serialisation
{
  template <typename T>
  concept ArchiveBuiltin = Arithmetic<T> || SameType<T, byte> || SameType<T, string>;

  template <typename T>
  concept NonArchiveBuiltin = !ArchiveBuiltin<T>;
}