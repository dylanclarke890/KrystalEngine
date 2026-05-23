#pragma once

#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/SkipInputError.hpp"

namespace Krys::Text::Handlers
{
  /// @brief An error handler that simply skips bad input on error, and performs no replacement.
  /// @remarks If a sequence of text depends on the state of the previous text, and there is an illegal
  /// sequence in the middle of such a sequence, using their error handler can cause a cascade of failures
  /// as the `state` may not be prepared for the new input; therefore, it may skip more than a user of this
  /// error handler might expect. This is something that may happen due to shift state issues (e.g., when an
  /// encoding is not self-synchronizing.
  class SkipHandler
  {
  public:
    /// @brief Skips over any input that may produce an error.
    /// @param[in] encoding The Encoding that experienced the error.
    /// @param[in] result The current state of the encode operation.
    /// @param[in] inputProgress How much input was (potentially irreversibly) read from the input range
    /// before undergoing the attempted encode operation.
    /// @param[in] outputProgress How much output was (potentially irreversibly) written to the output
    /// range before undergoing the attempted encode operation.
    template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
    constexpr auto operator()(const TEncoding &encoding, TResult &&result,
                              const TInputProgress &inputProgress,
                              const TOutputProgress &outputProgress) const
      noexcept(
        NoThrowSkipInputError<const TEncoding &, TResult, const TInputProgress &, const TOutputProgress &>)
    {
      if (result.ErrorCode != EncodingError::InsufficientOutputSpace)
      {
        result.ErrorCode = EncodingError::OK;
      }
      return ::Krys::Text::SkipInputError(encoding, std::forward<TResult>(result), inputProgress,
                                          outputProgress);
    }
  };

  /// @brief An instance of SkipHandler for ease of use.
  constexpr inline SkipHandler Skip = {};
}
