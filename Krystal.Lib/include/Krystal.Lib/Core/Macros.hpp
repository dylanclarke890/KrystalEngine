#pragma once

#include "Krystal.Lib/Detection/Compiler.hpp"

namespace Krys
{
#define MOVE_SWAP(ClassName)                                                                                 \
  ClassName(ClassName &&other) noexcept                                                                      \
  {                                                                                                          \
    Swap(other);                                                                                             \
  }                                                                                                          \
                                                                                                             \
  ClassName &operator=(ClassName &&other) noexcept                                                           \
  {                                                                                                          \
    if (this != &other)                                                                                      \
    {                                                                                                        \
      Swap(other);                                                                                           \
    }                                                                                                        \
    return *this;                                                                                            \
  }                                                                                                          \
  void swap(ClassName &other) noexcept                                                                       \
  {                                                                                                          \
    Swap(other);                                                                                             \
  }

#define KRYS_CONCAT_IMPL(x, y) x##y
#define KRYS_CONCAT(x, y) KRYS_CONCAT_IMPL(x, y)

#define KRYS_STRINGIFY_IMPL(x) #x
#define KRYS_STRINGIFY(x) KRYS_STRINGIFY_IMPL(x)
}