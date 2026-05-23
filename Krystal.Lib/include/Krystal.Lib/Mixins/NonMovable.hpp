#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys
{
  /// @brief A mixin class that makes a class non-movable.
  template <class T>
  class KRYS_EMPTY_BASE_CLASS NonMovable
  {
  public:
    NonMovable(const NonMovable &&) = delete;
    T &operator=(const T &&) = delete;

  protected:
    NonMovable() = default;
    ~NonMovable() = default;
  };
}