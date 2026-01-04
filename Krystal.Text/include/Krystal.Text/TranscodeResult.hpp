#pragma once

#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/EncodingError.hpp"
#include "Krystal.Text/Impl/ResultTypeConstraints.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <array>
#include <cstddef>
#include <utility>

namespace Krys
{
  /// @brief The result of transcoding operations (such as transcode) that specifically do not include
  /// a reference to the state.
  template <typename TInput, typename TOutput>
  class StatelessTranscodeResult
  {
  public:
    /// @brief The reconstructed input_view object, with its .begin() incremented by the number of code units
    /// successfully read (can be identical to .begin() on original range on failure).
    TInput Input;

    /// @brief The reconstructed output_view object, with its .begin() incremented by the number of code units
    /// successfully written (can be identical to .begin() on original range on failure).
    TOutput Output;

    /// @brief The kind of error that occured, if any.
    EncodingError ErrorCode;

    /// @brief Whether or not the error handler was invoked, regardless of if the ErrorCode is set or not set
    /// to EncodingError::OK.
    std::size_t ErrorCount;

    /// @brief Constructs a StatelessTranscodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput>
    requires CopyableResultType<StatelessTranscodeResult, TInput, TArgInput, TOutput, TArgOutput>
    constexpr StatelessTranscodeResult(const StatelessTranscodeResult<TArgInput, TArgOutput> &other) // cf
      noexcept(NoThrowCopyableResultType<StatelessTranscodeResult, TInput, TArgInput, TOutput, TArgOutput>)
        : Input(other.Input), Output(other.Output), ErrorCode(other.ErrorCode), ErrorCount(other.ErrorCount)
    {
    }

    /// @brief Constructs a StatelessTranscodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput>
    requires MovableResultType<StatelessTranscodeResult, TInput, TArgInput, TOutput, TArgOutput>
    constexpr StatelessTranscodeResult(StatelessTranscodeResult<TArgInput, TArgOutput> &&other) // cf
      noexcept(NoThrowMovableResultType<StatelessTranscodeResult, TInput, TArgInput, TOutput, TArgOutput>)
        : Input(std::move(other.Input)), Output(std::move(other.Output)), ErrorCode(other.ErrorCode),
          ErrorCount(other.ErrorCount)
    {
    }

    /// @brief Constructs a StatelessTranscodeResult, defaulting the error code to
    /// EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    template <typename TArgInput, typename TArgOutput>
    constexpr StatelessTranscodeResult(TArgInput &&input, TArgOutput &&output,
                                       EncodingError errorCode = EncodingError::OK) // cf
      noexcept(noexcept(StatelessTranscodeResult(std::forward<TArgInput>(input),
                                                 std::forward<TArgOutput>(output), errorCode,
                                                 errorCode != EncodingError::OK)))
        : StatelessTranscodeResult(std::forward<TArgInput>(input), std::forward<TArgOutput>(output),
                                   errorCode, errorCode != EncodingError::OK)
    {
    }

    /// @brief Constructs a StatelessTranscodeResult with the provided parameters and
    /// information, including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput>
    constexpr StatelessTranscodeResult(TArgInput &&input, TArgOutput &&output, EncodingError errorCode,
                                       std::size_t errorCount) // cf
      noexcept(NoThrowConstructible<TInput, TArgInput> && NoThrowConstructible<TOutput, TArgOutput>)
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

  /// @brief The result of transcoding operations (such as transcode with the state argument provided).
  template <typename TInput, typename TOutput, typename TFromState, typename TToState>
  class PivotlessTranscodeResult : public StatelessTranscodeResult<TInput, TOutput>
  {
  private:
    using TBase = StatelessTranscodeResult<TInput, TOutput>;

  public:
    /// @brief A reference to the state of the associated Encoding used for decoding input code units to
    /// intermediate code points.
    ReferenceWrapper<TFromState> FromState;

    /// @brief A reference to the state of the associated Encoding used for encoding intermediate code points
    /// to code units.
    ReferenceWrapper<TToState> ToState;

    /// @brief Constructs a PivotlessTranscodeResult from a previous PivotlessTranscodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
    requires CopyableResultType<PivotlessTranscodeResult, TInput, TArgInput, TOutput, TArgOutput, TFromState,
                                TArgFromState, TToState, TArgToState>
    constexpr PivotlessTranscodeResult(
      const PivotlessTranscodeResult<TArgInput, TArgOutput, TArgFromState, TArgToState> &other) // cf
      noexcept(NoThrowCopyableResultType<PivotlessTranscodeResult, TInput, TArgInput, TOutput, TArgOutput,
                                         TFromState, TArgFromState, TToState, TArgToState>)
        : TBase(other.Input, other.Output, other.ErrorCode, other.ErrorCount), FromState(other.FromState),
          ToState(other.ToState)
    {
    }

    /// @brief Constructs a PivotlessTranscodeResult from a previous PivotlessTranscodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
    requires MovableResultType<PivotlessTranscodeResult, TInput, TArgInput, TOutput, TArgOutput, TFromState,
                               TArgFromState, TToState, TArgToState>
    constexpr PivotlessTranscodeResult(
      PivotlessTranscodeResult<TArgInput, TArgOutput, TArgFromState, TArgToState> &&other) // cf
      noexcept(NoThrowMovableResultType<PivotlessTranscodeResult, TInput, TArgInput, TOutput, TArgOutput,
                                        TFromState, TArgFromState, TToState, TArgToState>())
        : TBase(std::move(other.Input), std::move(other.Output), other.ErrorCode, other.ErrorCount),
          FromState(other.FromState), ToState(other.ToState)
    {
    }

    /// @brief Constructs a PivotlessTranscodeResult, defaulting the error code to
    /// EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] fromState The state related to the "From Encoding" that performed the decode half of the
    /// operation.
    /// @param[in] toState The state related to the "To Encoding" that performed the encode half of the
    /// operation.
    /// @param[in] errorCode The error code for the transcoding operation, taken as the first of either the
    /// encode or decode operation that failed.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
    constexpr PivotlessTranscodeResult(TArgInput &&input, TArgOutput &&output, TArgFromState &&fromState,
                                       TArgToState &&toState, EncodingError errorCode = EncodingError::OK)
        : PivotlessTranscodeResult(std::forward<TArgInput>(input), std::forward<TArgOutput>(output),
                                   std::forward<TArgFromState>(fromState), std::forward<TArgToState>(toState),
                                   errorCode, errorCode != EncodingError::OK ? 1uz : 0uz)
    {
    }

    /// @brief Constructs a PivotlessTranscodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] fromState The state related to the "From Encoding" that performed the decode half of the
    /// operation.
    /// @param[in] toState The state related to the "To Encoding" that performed the encode half of the
    /// operation.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the encode
    /// or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
    constexpr PivotlessTranscodeResult(TArgInput &&input, TArgOutput &&output, TArgFromState &&fromState,
                                       TArgToState &&toState, EncodingError errorCode, std::size_t errorCount)
        : TBase(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), errorCode, errorCount),
          FromState(std::forward<TArgFromState>(fromState)), ToState(std::forward<TArgToState>(toState))
    {
    }
  };

  /// @brief The result of low-level transcoding operations (such as TranscodeInto with the pivot
  /// provided as an argument).
  template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
  class TranscodeResult : public PivotlessTranscodeResult<TInput, TOutput, TFromState, TToState>
  {
  private:
    using TBase = PivotlessTranscodeResult<TInput, TOutput, TFromState, TToState>;

  public:
    /// @brief The range used to hold the intermediate pivot transcoding units.
    KRYS_NO_UNIQUE_ADDRESS TPivot Pivot;

    /// @brief The kind of error that occured, if any, for the intermediate pivot.
    EncodingError PivotErrorCode;

    /// @brief Whether or not the error handler for the pivot point was invoked, regardless of if the
    /// `PivotErrorCode` is set or not set to EncodingError::OK.
    std::size_t PivotErrorCount;

    /// @brief Constructs a PivotlessTranscodeResult from a previous PivotlessTranscodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
              typename TArgPivot>
    requires CopyableResultType<TranscodeResult, TInput, TArgInput, TOutput, TArgOutput, TFromState,
                                TArgFromState, TToState, TArgToState, TPivot, TArgPivot>
    constexpr TranscodeResult(
      const TranscodeResult<TArgInput, TArgOutput, TArgFromState, TArgToState, TArgPivot> &other) // cf
      noexcept(NoThrowCopyableResultType<TranscodeResult, TInput, TArgInput, TOutput, TArgOutput, TFromState,
                                         TArgFromState, TToState, TArgToState, TPivot, TArgPivot>())
        : TBase(other.Input, other.Output, other.ErrorCode, other.ErrorCount, other.FromState, other.ToState),
          Pivot(other.Pivot), PivotErrorCode(other.PivotErrorCode), PivotErrorCount(other.PivotErrorCount)
    {
    }

    /// @brief Constructs a PivotlessTranscodeResult from a previous PivotlessTranscodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
              typename TArgPivot>
    requires MovableResultType<TranscodeResult, TInput, TArgInput, TOutput, TArgOutput, TFromState,
                               TArgFromState, TToState, TArgToState, TPivot, TArgPivot>
    constexpr TranscodeResult(
      TranscodeResult<TArgInput, TArgOutput, TArgFromState, TArgToState, TArgPivot> &&other) // cf
      noexcept(NoThrowMovableResultType<TranscodeResult, TInput, TArgInput, TOutput, TArgOutput, TFromState,
                                        TArgFromState, TToState, TArgToState, TPivot, TArgPivot>())
        : TBase(std::move(other.Input), std::move(other.Output), other.ErrorCode, other.ErrorCount,
                other.FromState, other.ToState),
          Pivot(std::move(other.Pivot)), PivotErrorCode(std::move(other.PivotErrorCode)),
          PivotErrorCount(std::move(other.PivotErrorCount))
    {
    }

    /// @brief Constructs a PivotlessTranscodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] fromState The state related to the "From Encoding" that performed the decode half of the
    /// operation.
    /// @param[in] toState The state related to the "To Encoding" that performed the encode half of the
    /// operation.
    /// @param[in] errorCode The error code for the transcode operation, taken as the first of either the
    /// encode or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    /// @param[in] pivot The pivot for this transcode operation.
    /// @param[in] pivotErrorCode The error code for the decode step of the transcode oepration, if it failed.
    /// @param[in] pivotErrorCount Whether or not an error was handled during the decode step of the transcode
    /// operation. Some error handlers are corrective (see Handlers::ReplacementHandler), and so the error
    /// code is not enough to determine if the handler was invoked. This allows the value to be provided
    /// directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
              typename TArgPivot>
    constexpr TranscodeResult(TArgInput &&input, TArgOutput &&output, TArgFromState &&fromState,
                              TArgToState &&toState, EncodingError errorCode, std::size_t errorCount,
                              TArgPivot &&pivot, EncodingError pivotErrorCode, std::size_t pivotErrorCount)
        : TBase(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), fromState, toState,
                errorCode, errorCount),
          Pivot(std::forward<TArgPivot>(pivot)), PivotErrorCode(std::move(pivotErrorCode)),
          PivotErrorCount(std::move(pivotErrorCount))
    {
    }
  };

  namespace Impl
  {
    template <typename TInput, typename TOutput, typename TFromState, typename TToState>
    constexpr StatelessTranscodeResult<TInput, TOutput>
      SliceToStateless(PivotlessTranscodeResult<TInput, TOutput, TFromState, TToState> &&result) noexcept(
        NoThrowConstructible<StatelessTranscodeResult<TInput, TOutput>,
                             StatelessTranscodeResult<TInput, TOutput>>)
    {
      return std::move(result);
    }

    template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
    constexpr PivotlessTranscodeResult<TInput, TOutput, TFromState, TToState>
      SliceToPivotless(TranscodeResult<TInput, TOutput, TFromState, TToState, TPivot> &&result) noexcept(
        NoThrowConstructible<PivotlessTranscodeResult<TInput, TOutput, TFromState, TToState>,
                             PivotlessTranscodeResult<TInput, TOutput, TFromState, TToState>>)
    {
      return std::move(result);
    }

    template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
    constexpr StatelessTranscodeResult<TInput, TOutput> SliceToStatelessPivotless(
      TranscodeResult<TInput, TOutput, TFromState, TToState, TPivot>
        &&result) noexcept(NoThrowConstructible<StatelessTranscodeResult<TInput, TOutput>,
                                                StatelessTranscodeResult<TInput, TOutput>>)
    {
      return std::move(result);
    }

    template <typename TInput, typename TOutput, typename TDesiredOutput, typename TFromState,
              typename TToState>
    constexpr auto ReplaceTranscodeResultOutputNoState(
      PivotlessTranscodeResult<TInput, TOutput, TFromState, TToState> &&result,
      TDesiredOutput
        &&desiredOutput) noexcept(NoThrowConstructible<StatelessTranscodeResult<TInput, TOutput>, TInput &&,
                                                       TDesiredOutput, EncodingError, std::size_t>)
    {
      using TResult = StatelessTranscodeResult<TInput, remove_cvref_t<TDesiredOutput>>;
      return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput), result.ErrorCode,
                     result.ErrorCount);
    }

    template <typename TInput, typename TOutput, typename TFromState, typename TToState,
              typename TDesiredOutput>
    constexpr auto ReplaceTranscodeResultOutput(
      PivotlessTranscodeResult<TInput, TOutput, TFromState, TToState> &&result,
      TDesiredOutput
        &&desiredOutput) noexcept(NoThrowConstructible<PivotlessTranscodeResult<TInput, TOutput, TFromState,
                                                                                TToState>,
                                                       TInput &&, TDesiredOutput, TFromState &, TToState &,
                                                       EncodingError, std::size_t>)
    {
      using TResult = PivotlessTranscodeResult<TInput, remove_cvref_t<TDesiredOutput>, TFromState, TToState>;
      return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput), result.FromState,
                     result.ToState, result.ErrorCode, result.ErrorCount);
    }

    template <typename TInputRange, typename _OutputRange, typename TFromState, typename TToState>
    using reconstruct_transcode_result_t =
      PivotlessTranscodeResult<Krys::Ranges::range_reconstruct_t<TInputRange>,
                               Krys::Ranges::range_reconstruct_t<_OutputRange>, TFromState, TToState>;

    template <typename TInputRange, typename _OutputRange, typename TFromState, typename TToState,
              typename TPivot>
    using reconstruct_pivot_transcode_result_t =
      TranscodeResult<Krys::Ranges::range_reconstruct_t<TInputRange>,
                      Krys::Ranges::range_reconstruct_t<_OutputRange>, TFromState, TToState,
                      Krys::Ranges::range_reconstruct_t<TPivot>>;
  }
}
