#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtrV1/RefCounted.hpp"

namespace Krys
{
  template <typename T>
  using WeakRef = WeakReference<T>;
}