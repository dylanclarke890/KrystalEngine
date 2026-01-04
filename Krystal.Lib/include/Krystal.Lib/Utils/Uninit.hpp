#pragma once

#include "Krystal.Lib/Utils/Unwrap.hpp"
#include <utility>

namespace Krys
{
  /// @brief A class for holding a value inside of an unnamed union which is composed of two objects, one of
  /// `char` and one of `T`.
  template <typename T>
  class alignas(T) Uninit
  {
  public:
    /// @brief Constructs an empty placeholder.
    constexpr Uninit() : placeholder()
    {
    }

    /// @brief An empty destructor. Required, as there is a union object present.
    ~Uninit()
    {
    }

    /// @brief Constructs the `value` from the given arguments
    /// @param[in] args The arguments to construct `value` with.
    template <typename... TArgs>
    constexpr Uninit(std::in_place_t, TArgs &&...args) : value(std::forward<TArgs>(args)...)
    {
    }

    /// @brief Extension point for returning the value inside of this uninitialized type.
    friend T &Unwrap(Uninit &wrappedValue) noexcept
    {
      return wrappedValue.value;
    }

    /// @brief Extension point for returning the value inside of this uninitialized type.
    friend const T &Unwrap(const Uninit &wrappedValue) noexcept
    {
      return wrappedValue.value;
    }

    /// @brief Extension point for returning the value inside of this uninitialized type.
    friend T &&Unwrap(Uninit &&wrappedValue) noexcept
    {
      return std::move(wrappedValue.value);
    }

    union
    {
      /// @brief Placeholder empty value for default / empty  initialization, esp. with arrays.
      char placeholder;

      /// @brief Actual value.
      T value;
    };
  };
}
