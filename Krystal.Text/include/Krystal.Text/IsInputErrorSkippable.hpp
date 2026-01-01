#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys
{
  /// @brief Whether or not the given `TEncoding` has a function called `SkipInputError` that takes the
  /// given `TResult` type with the given `TInputProgress` and `TOutputProgress` types.
  /// @tparam TEncoding The encoding that may contain the SkipInputError function.
  /// @tparam TResult The result type to check if the input is callable.
  /// @tparam TInputProgress The input progress type passed in to the error handler to be forwarded to the
  /// skip input error.
  /// @tparam TOutputProgress The output progress type passed in to the error handler to be forwarded to the
  /// skip input error.
  /// @remarks This is used by ReplacementHandler and SkipHandler to pass over
  /// malformed input when it happens.
  template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
  concept HasSkipInputError = requires {
    std::declval<TEncoding>().SkipInputError(std::declval<TResult>(), std::declval<TInputProgress>(),
                                             std::declval<TOutputProgress>());
  };
}
