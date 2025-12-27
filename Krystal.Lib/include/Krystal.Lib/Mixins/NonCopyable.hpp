#pragma once

namespace Krys
{
  /// @brief A mixin class that makes a class non-copyable.
  template <class T>
  class NonCopyable
  {
  public:
    NonCopyable(const NonCopyable &) = delete;
    T &operator=(const T &) = delete;

  protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
  };
}