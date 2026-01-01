#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/Impl/ResultTypeConstraints.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <array>
#include <cstddef>
#include <functional>
#include <system_error>
#include <utility>

namespace Krys
{
  /// @brief The result of all encode operations from encoding objects and higher-level calls,
  template <typename TInput, typename TOutput>
  class StatelessEncodeResult
  {
  public:
    /// @brief The reconstructed input_view object, with its .begin() incremented by the number of code
    /// units successfully read (can be identical to .begin() on original range on failure).
    TInput Input;

    /// @brief The reconstructed output_view object, with its .begin() incremented by the number of code
    /// units successfully written (can be identical to .begin() on original range on failure).
    TOutput Output;

    /// @brief The kind of error that occured, if any.
    EncodingError ErrorCode;

    /// @brief Whether or not the error handler was invoked, regardless of if the ErrorCode is set or not
    /// set to EncodingError::OK.
    std::size_t ErrorCount;

    /// @brief Constructs a EncodeResult with the provided parameters and information, including whether or
    /// not an error was handled.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput>
    requires(Impl::ResultTypeCopyConstraint<StatelessEncodeResult, TInput, TArgInput, TOutput, TArgOutput>())
    constexpr StatelessEncodeResult(const StatelessEncodeResult<TArgInput, TArgOutput> &other) noexcept(
      Impl::ResultTypeCopyNoexcept<StatelessEncodeResult, TInput, TArgInput, TOutput, TArgOutput>())
        : Input(other.Input), Output(other.Output), ErrorCode(other.ErrorCode), ErrorCount(other.ErrorCount)
    {
    }

    /// @brief Constructs a EncodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput>
    requires(Impl::ResultTypeMoveConstraint<StatelessEncodeResult, TInput, TArgInput, TOutput, TArgOutput>())
    constexpr StatelessEncodeResult(StatelessEncodeResult<TArgInput, TArgOutput> &&other) noexcept(
      Impl::ResultTypeMoveNoexcept<StatelessEncodeResult, TInput, TArgInput, TOutput, TArgOutput>())
        : Input(std::move(other.Input)), Output(std::move(other.Output)), ErrorCode(other.ErrorCode),
          ErrorCount(other.ErrorCount)
    {
    }

    /// @brief Constructs a EncodeResult, defaulting the error code to EncodingError::OK if not provided.
    /// @param[in] input The Input range to store.
    /// @param[in] output The Output range to store.
    /// @param[in] errorCode The error code for the decoding opertion, if any.
    template <typename TArgInput, typename TArgOutput>
    constexpr StatelessEncodeResult(
      TArgInput &&input, TArgOutput &&output,
      EncodingError errorCode =
        EncodingError::OK) noexcept(noexcept(StatelessEncodeResult(std::forward<TArgInput>(input),
                                                                   std::forward<TArgOutput>(output),
                                                                   errorCode,
                                                                   errorCode != EncodingError::OK)))
        : StatelessEncodeResult(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), errorCode,
                                errorCode != EncodingError::OK ? 1uz : 0uz)
    {
    }

    /// @brief Constructs a EncodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The Input range to store.
    /// @param[in] output The Output range to store.
    /// @param[in] errorCode The error code for the encode operation, if any.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective
    /// (see ReplacementHandler), and so the error code is not enough to determine if the handler was invoked.
    /// This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput>
    constexpr StatelessEncodeResult(
      TArgInput &&input, TArgOutput &&output, EncodingError errorCode,
      std::size_t errorCount) noexcept(NoThrowConstructible<TInput, TArgInput>
                                       && NoThrowConstructible<TOutput, TArgOutput>)
        : Input(std::forward<TArgInput>(input)), Output(std::forward<TArgOutput>(output)),
          ErrorCode(errorCode), ErrorCount(errorCount)
    {
    }

    /// @brief Whether or not any errors were handled.
    /// @returns Simply checks whether `ErrorCount` is greater than 0.
    constexpr bool ErrorsWereHandled() const noexcept
    {
      return this->ErrorCount > 0;
    }
  };

  /// @brief The result of all encode operations from encoding objects and higher-level calls.
  template <typename TInput, typename TOutput, typename TState>
  class EncodeResult : public StatelessEncodeResult<TInput, TOutput>
  {
  private:
    using Base = StatelessEncodeResult<TInput, TOutput>;

  public:
    /// @brief The State of the associated Encoding used for decoding Input code points to code units.
    ReferenceWrapper<TState> State;

    /// @brief Constructs a EncodeResult from a previous EncodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgState>
    requires(Impl::ResultTypeCopyConstraint<EncodeResult, TInput, TArgInput, TOutput, TArgOutput, TState,
                                            TArgState>())
    constexpr EncodeResult(const EncodeResult<TArgInput, TArgOutput, TArgState> &other) noexcept(
      Impl::ResultTypeCopyNoexcept<EncodeResult, TInput, TArgInput, TOutput, TArgOutput, TState, TArgState>())
        : Base(other.Input, other.Output, other.ErrorCode, other.ErrorCount), State(other.State)
    {
    }

    /// @brief Constructs a EncodeResult from a previous EncodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgState>
    requires(Impl::ResultTypeMoveConstraint<EncodeResult, TInput, TArgInput, TOutput, TArgOutput, TState,
                                            TArgState>())
    constexpr EncodeResult(EncodeResult<TArgInput, TArgOutput, TArgState> &&other) noexcept(
      Impl::ResultTypeMoveNoexcept<EncodeResult, TInput, TArgInput, TOutput, TArgOutput, TState, TArgState>())
        : Base(std::move(other.Input), std::move(other.Output), other.ErrorCode, other.ErrorCount),
          State(other.State)
    {
    }

    /// @brief Constructs a EncodeResult, defaulting the error code to EncodingError::OK if not provided.
    /// @param[in] input The Input range to store.
    /// @param[in] output The Output range to store.
    /// @param[in] state The State related to the Encoding that performed the encode operation.
    /// @param[in] errorCode The error code for the decoding opertion, if any.
    template <typename TArgInput, typename TArgOutput, typename TArgState>
    constexpr EncodeResult(TArgInput &&input, TArgOutput &&output, TArgState &&state,
                           EncodingError errorCode = EncodingError::OK)
        : EncodeResult(std::forward<TArgInput>(input), std::forward<TArgOutput>(output),
                       std::forward<TArgState>(state), errorCode, errorCode != EncodingError::OK ? 1uz : 0uz)
    {
    }

    /// @brief Constructs a EncodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The Input range to store.
    /// @param[in] output The Output range to store.
    /// @param[in] state The State related to the Encoding that performed the encode operation.
    /// @param[in] errorCode The error code for the encode operation, if any.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective
    /// (see ReplacementHandler), and so the error code is not enough to determine if the handler was invoked.
    /// This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput, typename TArgState>
    constexpr EncodeResult(TArgInput &&input, TArgOutput &&output, TArgState &&state, EncodingError errorCode,
                           std::size_t errorCount) noexcept
        : Base(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), errorCode, errorCount),
          State(std::forward<TArgState>(state))
    {
    }
  };

  /// @brief A type alias to produce a span-containing encode result type. Useful for end-users with fairly
  /// standard, pointer-based buffer usages.
  template <typename TEncoding>
  using SpanEncodeResultFor =
    EncodeResult<Span<const code_point_t<TEncoding>>, Span<code_unit_t<TEncoding>>, EncodeState<TEncoding>>;

  /// @brief A type alias to produce a concrete error handler for the encoding result of the specified
  /// `TEncoding` type.
  /// @tparam TEncoding The encoding to base this error handler off of.
  /// @tparam TFunction The template function type that will be used as the base type to insert the function
  /// signature into.
  template <typename TEncoding, template <class...> class TFunction = std::function>
  using BasicEncodeErrorHandlerFor = TFunction<SpanEncodeResultFor<TEncoding>(
    const TEncoding &, SpanEncodeResultFor<TEncoding>, Span<const code_point_t<TEncoding>>)>;

  namespace Impl
  {
    template <typename TInput, typename TOutput, typename TState>
    constexpr StatelessEncodeResult<TInput, TOutput>
      SliceToStatelessEncode(EncodeResult<TInput, TOutput, TState> &&result) noexcept(
        NoThrowConstructible<StatelessEncodeResult<TInput, TOutput>, StatelessEncodeResult<TInput, TOutput>>)
    {
      return std::move(result);
    }

    template <typename TInput, typename TOutput, typename TState, typename TDesiredOutput>
    constexpr StatelessEncodeResult<TInput, remove_cvref_t<TDesiredOutput>> ReplaceEncodeResultOutputNoState(
      EncodeResult<TInput, TOutput, TState> &&result,
      TDesiredOutput
        &&desiredOutput) noexcept(NoThrowConstructible<StatelessEncodeResult<TInput, TOutput>, TInput &&,
                                                       TDesiredOutput, EncodingError, std::size_t>)
    {
      using TResult = StatelessEncodeResult<TInput, remove_cvref_t<TDesiredOutput>>;
      return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput), result.ErrorCode,
                     result.ErrorCount);
    }

    template <typename TInput, typename TOutput, typename TState, typename TDesiredOutput>
    constexpr EncodeResult<TInput, remove_cvref_t<TDesiredOutput>, TState> ReplaceEncodeResultOutput(
      EncodeResult<TInput, TOutput, TState> &&result,
      TDesiredOutput
        &&desiredOutput) noexcept(NoThrowConstructible<EncodeResult<TInput, TOutput, TState>, TInput &&,
                                                       TDesiredOutput, TState &, EncodingError, std::size_t>)
    {
      using TResult = EncodeResult<TInput, remove_cvref_t<TDesiredOutput>, TState>;
      return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput), result.State,
                     result.ErrorCode, result.ErrorCount);
    }

    template <typename TInputRange, typename TOutputRange, typename TState>
    using reconstruct_encode_result_t = EncodeResult<Krys::Ranges::range_reconstruct_t<TInputRange>,
                                                     Krys::Ranges::range_reconstruct_t<TOutputRange>, TState>;

    template <typename TInputRange, typename TOutputRange, typename TState, typename TInFirst,
              typename TInLast, typename TOutFirst, typename TOutLast, typename TArgState>
    constexpr decltype(auto) ReconstructStatelessEncodeResult(TInFirst &&inFirst, TInLast &&inLast,
                                                              TOutFirst &&outFirst, TOutLast &&outLast,
                                                              TArgState &&state, EncodingError errorCode,
                                                              std::size_t errorCount)
    {
      decltype(auto) inRange = Krys::Ranges::reconstruct(
        std::in_place_type<TInputRange>, std::forward<TInFirst>(inFirst), std::forward<TInLast>(inLast));
      decltype(auto) outRange = Krys::Ranges::reconstruct(
        std::in_place_type<TOutputRange>, std::forward<TOutFirst>(outFirst), std::forward<TOutLast>(outLast));
      return EncodeResult<TInputRange, TOutputRange, TState>(
        std::forward<decltype(inRange)>(inRange), std::forward<decltype(outRange)>(outRange),
        std::forward<TArgState>(state), errorCode, errorCount);
    }

    template <typename TInputRange, typename TOutputRange, typename TState, typename TInFirst,
              typename TInLast, typename TOutFirst, typename TOutLast, typename TArgState>
    constexpr decltype(auto) ReconstructStatelessEncodeResult(TInFirst &&inFirst, TInLast &&inLast,
                                                              TOutFirst &&outFirst, TOutLast &&outLast,
                                                              TArgState &&state,
                                                              EncodingError errorCode = EncodingError::OK)
    {
      return reconstruct_encode_result_t<TInputRange, TOutputRange, TState>(
        std::forward<TInFirst>(inFirst), std::forward<TInLast>(inLast), std::forward<TOutFirst>(outFirst),
        std::forward<TOutLast>(outLast), std::forward<TArgState>(state), errorCode);
    }
  }
}
