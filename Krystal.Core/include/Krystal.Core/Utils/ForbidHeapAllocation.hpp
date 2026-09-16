#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/Tags.hpp"

inline void *operator new(size_t, krys::NotNullTag, void *location)
{
  krys_assert(location);
  return location;
}

#define KRYS_FORBID_HEAP_ALLOCATION                                                                          \
private:                                                                                                     \
  void *operator new(size_t, void *) = delete;                                                               \
  void *operator new[](size_t, void *) = delete;                                                             \
  void *operator new(size_t) = delete;                                                                       \
  void *operator new[](size_t size) = delete;                                                                \
  void *operator new(size_t, krys::NotNullTag, void *) = delete;                                             \
  using _forceSemicolonForbidHeapAllocation = int

#define KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW                                                   \
public:                                                                                                      \
  void *operator new(size_t, krys::NotNullTag, void *location)                                               \
  {                                                                                                          \
    krys_assert(location);                                                                                   \
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