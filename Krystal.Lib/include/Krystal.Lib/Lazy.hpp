#pragma once

#include <optional>

namespace Krys
{
  /// @brief Thin wrapper class that allows for lazy initialization.
  /// @tparam T Any type. Usually one that has an expensive constructor that doesn't need to be invoked
  /// immediately.
  /// @tparam Fn A callable type that returns an instance of T. This is the function that will be called.
  template <typename T, typename Fn>
  class Lazy
  {
  private:
    std::optional<T> _value;
    Fn _func;

  public:
    Lazy(const Fn &func) : _func(func)
    {
    }

    /// @brief Returns a const reference to the lazily initialized value.
    const T &val()
    {
      Init();
      return *_value;
    }

    /// @brief Initialises the value if it hasn't been already.
    void Init() noexcept
    {
      if (!_value)
        _value = _func();
    }

    /// @brief Clears the value if it was set. Allows for `Init` to be called again.
    void Reset() noexcept
    {
      _value.reset();
    }
  };
}
