#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"

namespace Krys
{
  /// @brief Whether or not the given `T` is an error handler that can be ignored.
  /// @tparam T the Error Handling type to chec.
  /// @remarks An error handler type can mark itself as ignorable by using a `using` `assumeTValid` @c =
  /// `IntegralConstant`<bool, `value`> where `value` determines if the type's error handling callback
  /// can be ignored. This is what assumeTValid does. Being configurable means templated error
  /// handlers can select whether or not they should be ignorable based on compile time, safe conditions
  /// that you can make up (including checking Macros or other environment data as a means of determining
  /// whether or not validity should be ignored.) If this results in a type derived from `std::true_type`
  /// and the encoder object using it encounters an error, then it is Undefined Behavior what occurs
  /// afterwards.
  template <typename T>
  concept IsIgnorableErrorHandler = T::assumeTValid::value == true;
}
