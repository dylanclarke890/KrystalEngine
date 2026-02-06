#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys
{
  /// @brief A mixin class that makes a class non-copyable.
  template <class T>
  class KRYS_EMPTY_BASE_CLASS NonCopyable
  {
  public:
    NonCopyable(const NonCopyable &) = delete;
    T &operator=(const T &) = delete;

  protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
  };
}