#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/detail/ResultTypeConcepts.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/State.hpp"
#include <array>
#include <cstddef>
#include <functional>
#include <system_error>
#include <utility>

namespace Krys::Text
{
  /// @brief The result of all decode operations from encoding objects and higher-level calls.
  template <typename TInput, typename TOutput>
  class StatelessDecodeResult
  {
  public:
    /// @brief The reconstructed InputView object, with its .begin() incremented by the number of code
    /// units successfully read (can be identical to .begin() on original range on failure).
    TInput Input;

    /// @brief The reconstructed OutputView object, with its .begin() incremented by the number of code
    /// units successfully written (can be identical to .begin() on original range on failure).
    TOutput Output;

    /// @brief The kind of error that occured, if any.
    EncodingError ErrorCode;

    /// @brief The number of times an error occurred in the processed Input text.
    size_t ErrorCount;

    /// @brief Constructs a DecodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The Input range to store.
    /// @param[in] output The Output range to store.
    /// @param[in] errorCode The error code for the decode operation.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput>
    constexpr StatelessDecodeResult(TArgInput &&input, TArgOutput &&output, EncodingError errorCode,
                                    std::size_t errorCount) // cf
      noexcept(NoThrowConstructible<TInput, TArgInput> && NoThrowConstructible<TOutput, TArgOutput>)
        : Input(std::forward<TArgInput>(input)), Output(std::forward<TArgOutput>(output)),
          ErrorCode(errorCode), ErrorCount(errorCount)
    {
    }

    /// @brief Constructs a DecodeResult, defaulting the error code to EncodingError::OK if not provided.
    /// @param[in] input The Input range to store.
    /// @param[in] output The Output range to store.
    /// @param[in] errorCode The error code for the decoding opertion, if any.
    template <typename TArgInput, typename TArgOutput>
    constexpr StatelessDecodeResult(TArgInput &&input, TArgOutput &&output,
                                    EncodingError errorCode = EncodingError::OK) // cf
      noexcept(NoThrowConstructible<StatelessDecodeResult, TArgInput, TArgOutput, EncodingError, std::size_t>)
        : StatelessDecodeResult(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), errorCode,
                                errorCode != EncodingError::OK ? 1uz : 0uz)
    {
    }

    /// @brief Copy constructor.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput>
    requires CopyableResultType<StatelessDecodeResult, TInput, TArgInput, TOutput, TArgOutput>
    constexpr StatelessDecodeResult(const StatelessDecodeResult<TArgInput, TArgOutput> &other) // cf
      noexcept(NoThrowCopyableResultType<StatelessDecodeResult, TInput, TArgInput, TOutput, TArgOutput>)
        : Input(other.Input), Output(other.Output), ErrorCode(other.ErrorCode), ErrorCount(other.ErrorCount)
    {
    }

    /// @brief Move constructor.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput>
    requires MovableResultType<StatelessDecodeResult, TInput, TArgInput, TOutput, TArgOutput>
    constexpr StatelessDecodeResult(StatelessDecodeResult<TArgInput, TArgOutput> &&other) // cf
      noexcept(NoThrowMovableResultType<StatelessDecodeResult, TInput, TArgInput, TOutput, TArgOutput>)
        : Input(std::move(other.Input)), Output(std::move(other.Output)), ErrorCode(other.ErrorCode),
          ErrorCount(other.ErrorCount)
    {
    }

    /// @brief Whether or not any errors were handled.
    /// @returns Simply checks whether `ErrorCount` is greater than 0.
    KRYS_NODISCARD constexpr bool ErrorsWereHandled() const noexcept
    {
      return this->ErrorCount > 0;
    }
  };

  /// @brief The result of all decode operations from encoding objects and higher-level calls.
  template <typename TInput, typename TOutput, typename TState>
  class DecodeResult : public StatelessDecodeResult<TInput, TOutput>
  {
  private:
    using Base = StatelessDecodeResult<TInput, TOutput>;

  public:
    /// @brief The state of the associated Encoding used for decoding Input code units to code points.
    ReferenceWrapper<TState> State;

    /// @brief Constructs a DecodeResult from a previous DecodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgState>
    requires CopyableResultType<DecodeResult, TInput, TArgInput, TOutput, TArgOutput, TState, TArgState>
    constexpr DecodeResult(const DecodeResult<TArgInput, TArgOutput, TArgState> &other) // cf
      noexcept(
        NoThrowCopyableResultType<DecodeResult, TInput, TArgInput, TOutput, TArgOutput, TState, TArgState>)
        : Base(other.Input, other.Output, other.ErrorCode, other.ErrorCount), State(other.State)
    {
    }

    /// @brief Constructs a DecodeResult from a previous DecodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgState>
    requires MovableResultType<DecodeResult, TInput, TArgInput, TOutput, TArgOutput, TState, TArgState>
    constexpr DecodeResult(DecodeResult<TArgInput, TArgOutput, TArgState> &&other) // cf
      noexcept(
        NoThrowMovableResultType<DecodeResult, TInput, TArgInput, TOutput, TArgOutput, TState, TArgState>)
        : Base(std::move(other.Input), std::move(other.Output), other.ErrorCode, other.ErrorCount),
          State(other.State)
    {
    }

    /// @brief Constructs a DecodeResult, defaulting the error code to EncodingError::OK if not provided.
    /// @param[in] input The Input range to store.
    /// @param[in] output The Output range to store.
    /// @param[in] State The State related to the Encoding that performed the decode operation.
    /// @param[in] errorCode The error code for the decoding opertion, if any.
    template <typename TArgInput, typename TArgOutput, typename TArgState>
    constexpr DecodeResult(TArgInput &&input, TArgOutput &&output, TArgState &&state,
                           EncodingError errorCode = EncodingError::OK)
        : DecodeResult(std::forward<TArgInput>(input), std::forward<TArgOutput>(output),
                       std::forward<TArgState>(state), errorCode, errorCode != EncodingError::OK ? 1uz : 0uz)
    {
    }

    /// @brief Constructs a DecodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The Input range to store.
    /// @param[in] output The Output range to store.
    /// @param[in] state The state related to the Encoding that performed the decode operation.
    /// @param[in] errorCode The error code for the decode operation, taken as the first of either the
    /// decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective
    /// (Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput, typename TArgState>
    constexpr DecodeResult(TArgInput &&input, TArgOutput &&output, TArgState &&state, EncodingError errorCode,
                           std::size_t errorCount)
        : Base(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), errorCode, errorCount),
          State(std::forward<TArgState>(state))
    {
    }
  };

  /// @brief A type alias to produce a span-containing decode result type. Useful for end-users with fairly
  /// standard, pointer-Based buffer usages.
  template <typename TEncoding>
  using SpanDecodeResultFor = DecodeResult<Span<const code_unit_t<TEncoding>>, Span<code_point_t<TEncoding>>,
                                           decode_state_t<TEncoding>>;

  /// @brief A type alias to produce a concrete error handler for the encoding result of the specified
  /// `TEncoding` type.
  /// @tparam TEncoding The encoding to Base this error handler off of.
  /// @tparam TFunction The template function type that will be used as the Base type to insert the function
  /// signature into.
  template <typename TEncoding, template <class...> class TFunction = std::function>
  using BasicDecodeErrorHandlerFor = TFunction<SpanDecodeResultFor<TEncoding>(
    const TEncoding &, SpanDecodeResultFor<TEncoding>, Span<const code_unit_t<TEncoding>>)>;
}

namespace Krys::Text::detail_result
{
  template <typename TInput, typename TOutput, typename TState>
  constexpr StatelessDecodeResult<TInput, TOutput>
    SliceToStatelessDecode(DecodeResult<TInput, TOutput, TState> &&result) noexcept(
      NoThrowConstructible<StatelessDecodeResult<TInput, TOutput>, StatelessDecodeResult<TInput, TOutput>>)
  {
    return std::move(result);
  }

  template <typename TInput, typename TOutput, typename TDesiredOutput>
  constexpr StatelessDecodeResult<TInput, remove_cvref_t<TDesiredOutput>>
    ReplaceStatelessDecodeResultOutput(StatelessDecodeResult<TInput, TOutput> &&result,
                                       TDesiredOutput &&desiredOutput) // cf
    noexcept(NoThrowConstructible<StatelessDecodeResult<TInput, TOutput>, TInput &&, TDesiredOutput,
                                  EncodingError, std::size_t>)
  {
    using TResult = StatelessDecodeResult<TInput, remove_cvref_t<TDesiredOutput>>;
    return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput), result.ErrorCode,
                   result.ErrorCount);
  }

  template <typename TInput, typename TOutput, typename TState, typename TDesiredOutput>
  constexpr DecodeResult<TInput, remove_cvref_t<TDesiredOutput>, TState>
    ReplaceDecodeResultOutput(DecodeResult<TInput, TOutput, TState> &&result,
                              TDesiredOutput &&desiredOutput) // cf
    noexcept(NoThrowConstructible<DecodeResult<TInput, TOutput, TState>, TInput &&, TDesiredOutput, TState &,
                                  EncodingError, std::size_t>)
  {
    using TResult = DecodeResult<TInput, remove_cvref_t<TDesiredOutput>, TState>;
    return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput), result.State,
                   result.ErrorCode, result.ErrorCount);
  }

  template <typename TInputRange, typename TOutputRange, typename TState>
  using reconstruct_decode_result_t = DecodeResult<Krys::Ranges::range_reconstruct_t<TInputRange>,
                                                   Krys::Ranges::range_reconstruct_t<TOutputRange>, TState>;

  template <typename TInputRange, typename TOutputRange, typename TState, typename TInFirst, typename TInLast,
            typename TOutFirst, typename TOutLast, typename TArgState>
  constexpr decltype(auto) ReconstructStatelessDecodeResult(TInFirst &&inFirst, TInLast &&inLast,
                                                            TOutFirst &&outFirst, TOutLast &&outLast,
                                                            TArgState &&state, EncodingError errorCode,
                                                            std::size_t errorCount)
  {
    decltype(auto) inRange = ::Krys::Ranges::reconstruct(
      std::in_place_type<TInputRange>, std::forward<TInFirst>(inFirst), std::forward<TInLast>(inLast));
    decltype(auto) outRange = ::Krys::Ranges::reconstruct(
      std::in_place_type<TOutputRange>, std::forward<TOutFirst>(outFirst), std::forward<TOutLast>(outLast));
    return DecodeResult<TInputRange, TOutputRange, TState>(
      std::forward<decltype(inRange)>(inRange), std::forward<decltype(outRange)>(outRange),
      std::forward<TArgState>(state), errorCode, errorCount);
  }

  template <typename TInputRange, typename TOutputRange, typename TState, typename TInFirst, typename TInLast,
            typename TOutFirst, typename TOutLast, typename TArgState>
  constexpr decltype(auto) ReconstructStatelessDecodeResult(TInFirst &&inFirst, TInLast &&inLast,
                                                            TOutFirst &&outFirst, TOutLast &&outLast,
                                                            TArgState &&state,
                                                            EncodingError errorCode = EncodingError::OK)
  {
    return reconstruct_decode_result_t<TInputRange, TOutputRange, TState>(
      std::forward<TInFirst>(inFirst), std::forward<TInLast>(inLast), std::forward<TOutFirst>(outFirst),
      std::forward<TOutLast>(outLast), std::forward<TArgState>(state), errorCode);
  }
}