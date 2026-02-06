#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Mixins/NonMovable.hpp"

namespace Krys
{
  /// @brief A mixin class that makes a class non-copyable and non-movable.
  template <class T>
  class KRYS_EMPTY_BASE_CLASS NonCopyMovable
  {
  public:
    NonCopyMovable(const NonCopyMovable &) = delete;
    NonCopyMovable(const NonCopyMovable &&) = delete;

    T &operator=(const T &) = delete;
    T &operator=(const T &&) = delete;

  protected:
    NonCopyMovable() = default;
    ~NonCopyMovable() = default;
  };
}