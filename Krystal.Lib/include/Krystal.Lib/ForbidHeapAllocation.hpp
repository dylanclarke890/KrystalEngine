#pragma once

#include "Krystal.Lib/Core/Memory.hpp"
#include <cassert>

#define KRYS_FORBID_HEAP_ALLOCATION                                                                          \
private:                                                                                                     \
  void *operator new(size_t, void *) = delete;                                                               \
  void *operator new[](size_t, void *) = delete;                                                             \
  void *operator new(size_t) = delete;                                                                       \
  void *operator new[](size_t size) = delete;                                                                \
  void *operator new(size_t, NotNullTag, void *) = delete;                                                   \
  typedef int _thisIsHereToForceASemicolonAfterThisForbidHeapAllocationMacro

#define KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW                                                   \
public:                                                                                                      \
  void *operator new(size_t, NotNullTag, void *location)                                                     \
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
  typedef int _thisIsHereToForceASemicolonAfterThisForbidHeapAllocationAllowingPlacementNewMacro