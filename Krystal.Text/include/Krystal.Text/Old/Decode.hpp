#pragma once

namespace Krys
{
  template <typename TInput, typename TEncoding, typename TOutput, typename TErrorHandler, typename TState>
  constexpr auto Decode(
    TInput input,
    TEncoding &encoding,
    TOutput &output,
    TErrorHandler &errorHandler,
    TState &state) noexcept
  {
  }
}