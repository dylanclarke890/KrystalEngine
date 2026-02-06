#pragma once

#include "Krystal.Lib/Pointers/RefCounted/CompactPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"

namespace Krys
{
  template <typename T>
  using CompactRefPtr = RefPtr<T, CompactPtrTraits<T>>;
}
