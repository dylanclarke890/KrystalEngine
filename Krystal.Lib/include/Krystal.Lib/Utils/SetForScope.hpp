#pragma once

#include "Krystal.Lib/Core/Move.hpp"

namespace Krys
{
  /// @brief Useful for setting a variable to a new value only within a particular scope. An SetForScope<>
  /// object changes a variable to its original value upon destruction, making it an alternative to writing
  /// "var = false;" or "var = oldVal;" at all of a block's exit points.
  /// @note This should be obvious, but note that an SetForScope<> instance should have a shorter lifetime
  /// than its scopedVariable, to prevent invalid memory writes when the SetForScope<> object is destroyed.
  template <typename T>
  class SetForScope
  {
  private:
    T &_scopedVariable;
    T _originalValue;

  public:
    SetForScope(T &scopedVariable) noexcept : _scopedVariable(scopedVariable), _originalValue(scopedVariable)
    {
    }

    ~SetForScope() noexcept
    {
      _scopedVariable = Krys::Move(_originalValue);
    }

    template <typename U>
    SetForScope(T &scopedVariable, U &&newValue) noexcept : SetForScope(scopedVariable)
    {
      _scopedVariable = std::forward<U>(newValue);
    }

    template <typename U, typename V>
    SetForScope(T &scopedVariable, U &&newValue, V &&restoreValue) noexcept
        : _scopedVariable(scopedVariable), _originalValue(std::forward<V>(restoreValue))
    {
      _scopedVariable = std::forward<U>(newValue);
    }
  };
}