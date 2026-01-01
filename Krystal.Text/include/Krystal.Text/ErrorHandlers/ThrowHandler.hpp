#pragma once

#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include <exception>
#include <system_error>

namespace Krys::Handlers
{
  /// @brief An error handler that throws on any encode operation failure.
  /// @remarks This class absolutely should not be used unless the user is prepared to handle spurious
  /// failure, especially for text processing that deals with input vectors. This can result in many
  /// exceptions being thrown, which for resource-intensive applications could cause issues and result in
  /// Denial of Service by way of repeated, unhandled, and unexpected failure.
  class ThrowHandler
  {
  public:
    /// @brief Throws a EncodingError as an exception on an encode failure.
    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr EncodeResult<TInput, TOutput, TState>
      operator()(const TEncoding &, EncodeResult<TInput, TOutput, TState> result, const TInputProgress &,
                 const TOutputProgress &) const noexcept(false)
    {
      throw std::system_error(static_cast<int>(result.ErrorCode), EncodingCategory());
    }

    /// @brief Throws a EncodingError code as an exception on a decode failure.
    template <typename TEncoding, typename TInput, typename TOutput, typename TState, typename TInputProgress,
              typename TOutputProgress>
    constexpr DecodeResult<TInput, TOutput, TState>
      operator()(const TEncoding &, DecodeResult<TInput, TOutput, TState> result, const TInputProgress &,
                 const TOutputProgress &) const noexcept(false)
    {
      throw std::system_error(static_cast<int>(result.ErrorCode), EncodingCategory());
    }
  };

  /// @brief An instance of ThrowHandler for ease of use.
  constexpr inline ThrowHandler Throw = {};
}
