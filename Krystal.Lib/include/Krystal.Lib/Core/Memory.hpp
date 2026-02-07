#pragma once

#include <cassert>

namespace Krys
{
  enum NotNullTag
  {
    NotNull
  };
}

inline void *operator new(size_t, Krys::NotNullTag, void *location)
{
  assert(location);
  return location;
}