#pragma once

#include "Krystal.Lib/Pointers/RefCounted.hpp"

namespace Krys
{
  template <typename T>
  using WeakRef = WeakReference<T>;
}