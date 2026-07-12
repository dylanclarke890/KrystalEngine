#pragma once

#include "Krystal.Lib/Core/Tags.hpp"
#include <cassert>

inline void *operator new(size_t, Krys::NotNullTag, void *location)
{
  assert(location);
  return location;
}

#define KRYS_FORBID_HEAP_ALLOCATION                                                                          \
private:                                                                                                     \
  void *operator new(size_t, void *) = delete;                                                               \
  void *operator new[](size_t, void *) = delete;                                                             \
  void *operator new(size_t) = delete;                                                                       \
  void *operator new[](size_t size) = delete;                                                                \
  void *operator new(size_t, NotNullTag, void *) = delete;                                                   \
  using _forceSemicolonForbidHeapAllocation = int

#define KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW                                                   \
public:                                                                                                      \
  void *operator new(size_t, Krys::NotNullTag, void *location)                                               \
  {                                                                                                          \
    assert(location);                                                                                        \
    return location;                                                                                         \
  }                                                                                                          \
  void *operator new(size_t, void *location)                                                                 \
  {                                                                                                          \
    return location;                                                                                         \
  }                                                                                                          \
  void *operator new[](size_t, void *location)                                                               \
  {                                                                                                          \
    return location;                                                                                         \
  }                                                                                                          \
                                                                                                             \
private:                                                                                                     \
  void *operator new(size_t) = delete;                                                                       \
  void *operator new[](size_t size) = delete;                                                                \
  using _forceSemicolonForbidHeapAllocationAllowingPlacementNew = int