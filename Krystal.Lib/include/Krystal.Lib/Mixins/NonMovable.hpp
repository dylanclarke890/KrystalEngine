#pragma once

namespace Krys
{
  /// @brief A mixin class that makes a class non-movable.
  template <class T>
  class NonMovable
  {
  public:
    NonMovable(const NonMovable &&) = delete;
    T &operator=(const T &&) = delete;

  protected:
    NonMovable() = default;
    ~NonMovable() = default;
  };
}