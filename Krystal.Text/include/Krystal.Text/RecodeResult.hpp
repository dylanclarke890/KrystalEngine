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
  /// @brief The result of transcoding operations (such as recode) that specifically do not include
  /// a reference to the state.
  template <typename TInput, typename TOutput>
  class StatelessRecodeResult
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

    /// @brief Constructs a StatelessRecodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput>
    requires(Impl::ResultTypeCopyConstraint<StatelessRecodeResult, TInput, TArgInput, TOutput, TArgOutput>())
    constexpr StatelessRecodeResult(const StatelessRecodeResult<TArgInput, TArgOutput> &other) noexcept(
      Impl::ResultTypeCopyNoexcept<StatelessRecodeResult, TInput, TArgInput, TOutput, TArgOutput>())
        : Input(other.Input), Output(other.Output), ErrorCode(other.ErrorCode), ErrorCount(other.ErrorCount)
    {
    }

    /// @brief Constructs a StatelessRecodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput>
    requires(Impl::ResultTypeMoveConstraint<StatelessRecodeResult, TInput, TArgInput, TOutput, TArgOutput>())
    constexpr StatelessRecodeResult(StatelessRecodeResult<TArgInput, TArgOutput> &&other) noexcept(
      Impl::ResultTypeMoveNoexcept<StatelessRecodeResult, TInput, TArgInput, TOutput, TArgOutput>)
        : Input(std::move(other.Input)), Output(std::move(other.Output)), ErrorCode(other.ErrorCode),
          ErrorCount(other.ErrorCount)
    {
    }

    /// @brief Constructs a StatelessRecodeResult, defaulting the error code to
    /// EncodingError::OK if not provided.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the
    /// encode or decode operation that failed.
    template <typename TArgInput, typename TArgOutput>
    constexpr StatelessRecodeResult(
      TArgInput &&input, TArgOutput &&output,
      EncodingError errorCode =
        EncodingError::OK) noexcept(noexcept(StatelessRecodeResult(std::forward<TArgInput>(input),
                                                                   std::forward<TArgOutput>(output),
                                                                   errorCode,
                                                                   errorCode != EncodingError::OK)))
        : StatelessRecodeResult(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), errorCode,
                                errorCode != EncodingError::OK)
    {
    }

    /// @brief Constructs a StatelessRecodeResult with the provided parameters and
    /// information, including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the
    /// encode or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput>
    constexpr StatelessRecodeResult(
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

  /// @brief The result of transcoding operations (such as recode with the state argument provided).
  template <typename TInput, typename TOutput, typename TFromState, typename TToState>
  class PivotlessRecodeResult : public StatelessRecodeResult<TInput, TOutput>
  {
  private:
    using TBase = StatelessRecodeResult<TInput, TOutput>;

  public:
    /// @brief A reference to the state of the associated Encoding used for decoding input code units to
    /// intermediate code points.
    ReferenceWrapper<TFromState> FromState;

    /// @brief A reference to the state of the associated Encoding used for encoding intermediate code points
    /// to code units.
    ReferenceWrapper<TToState> ToState;

    /// @brief Constructs a PivotlessRecodeResult from a previous PivotlessRecodeResult.
    ///
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
    requires(Impl::ResultTypeCopyConstraint<PivotlessRecodeResult, TInput, TArgInput, TOutput, TArgOutput,
                                            TFromState, TArgFromState, TToState, TArgToState>())
    constexpr PivotlessRecodeResult(
      const PivotlessRecodeResult<TArgInput, TArgOutput, TArgFromState, TArgToState>
        &other) noexcept(Impl::ResultTypeCopyNoexcept<PivotlessRecodeResult, TInput, TArgInput, TOutput,
                                                      TArgOutput, TFromState, TArgFromState, TToState,
                                                      TArgToState>())
        : TBase(other.Input, other.Output, other.ErrorCode, other.ErrorCount), FromState(other.FromState),
          ToState(other.ToState)
    {
    }

    /// @brief Constructs a PivotlessRecodeResult from a previous PivotlessRecodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
    requires(Impl::ResultTypeMoveConstraint<PivotlessRecodeResult, TInput, TArgInput, TOutput, TArgOutput,
                                            TFromState, TArgFromState, TToState, TArgToState>())
    constexpr PivotlessRecodeResult(
      PivotlessRecodeResult<TArgInput, TArgOutput, TArgFromState, TArgToState>
        &&other) noexcept(Impl::ResultTypeMoveNoexcept<PivotlessRecodeResult, TInput, TArgInput, TOutput,
                                                       TArgOutput, TFromState, TArgFromState, TToState,
                                                       TArgToState>())
        : TBase(std::move(other.Input), std::move(other.Output), other.ErrorCode, other.ErrorCount),
          FromState(other.FromState), ToState(other.ToState)
    {
    }

    /// @brief Constructs a PivotlessRecodeResult, defaulting the error code to
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
    constexpr PivotlessRecodeResult(TArgInput &&input, TArgOutput &&output, TArgFromState &&fromState,
                                    TArgToState &&toState, EncodingError errorCode = EncodingError::OK)
        : PivotlessRecodeResult(
            std::forward<TArgInput>(input), std::forward<TArgOutput>(output),
            std::forward<TArgFromState>(fromState), std::forward<TArgToState>(toState), errorCode,
            errorCode != EncodingError::OK ? static_cast<std::size_t>(1) : static_cast<std::size_t>(0))
    {
    }

    /// @brief Constructs a PivotlessRecodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] fromState The state related to the "From Encoding" that performed the decode half of the
    /// operation.
    /// @param[in] toState The state related to the "To Encoding" that performed the encode half of the
    /// operation.
    /// @param[in] errorCode The error code for the encode operation, taken as the first of either the
    /// encode or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState>
    constexpr PivotlessRecodeResult(TArgInput &&input, TArgOutput &&output, TArgFromState &&fromState,
                                    TArgToState &&toState, EncodingError errorCode, std::size_t errorCount)
        : TBase(std::forward<TArgInput>(input), std::forward<TArgOutput>(output), errorCode, errorCount),
          FromState(std::forward<TArgFromState>(fromState)), ToState(std::forward<TArgToState>(toState))
    {
    }
  };

  /// @brief The result of low-level transcoding operations (such as recode_into with the pivot
  /// provided as an argument).
  template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
  class RecodeResult : public PivotlessRecodeResult<TInput, TOutput, TFromState, TToState>
  {
  private:
    using TBase = PivotlessRecodeResult<TInput, TOutput, TFromState, TToState>;

  public:
    /// @brief The range used to hold the intermediate pivot transcoding units.
    KRYS_NO_UNIQUE_ADDRESS TPivot Pivot;

    /// @brief The kind of error that occured, if any, for the intermediate pivot.
    EncodingError PivotErrorCode;

    /// @brief Whether or not the error handler for the pivot point was invoked, regardless of if the
    /// `PivotErrorCode` is set or not set to EncodingError::OK.
    std::size_t PivotErrorCount;

    /// @brief Constructs a PivotlessRecodeResult from a previous PivotlessRecodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
              typename TArgPivot>
    requires(Impl::ResultTypeCopyConstraint<RecodeResult, TInput, TArgInput, TOutput, TArgOutput, TFromState,
                                            TArgFromState, TToState, TArgToState, TPivot, TArgPivot>())
    constexpr RecodeResult(
      const RecodeResult<TArgInput, TArgOutput, TArgFromState, TArgToState, TArgPivot>
        &other) noexcept(Impl::ResultTypeCopyNoexcept<RecodeResult, TInput, TArgInput, TOutput, TArgOutput,
                                                      TFromState, TArgFromState, TToState, TArgToState,
                                                      TPivot, TArgPivot>())
        : TBase(other.Input, other.Output, other.ErrorCode, other.ErrorCount, other.FromState, other.ToState),
          Pivot(other.Pivot), PivotErrorCode(other.PivotErrorCode), PivotErrorCount(other.PivotErrorCount)
    {
    }

    /// @brief Constructs a PivotlessRecodeResult from a previous PivotlessRecodeResult.
    /// @param[in] other A different but related result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
              typename TArgPivot>
    requires(Impl::ResultTypeMoveConstraint<RecodeResult, TInput, TArgInput, TOutput, TArgOutput, TFromState,
                                            TArgFromState, TToState, TArgToState, TPivot, TArgPivot>())
    constexpr RecodeResult(
      RecodeResult<TArgInput, TArgOutput, TArgFromState, TArgToState, TArgPivot>
        &&other) noexcept(Impl::ResultTypeMoveNoexcept<RecodeResult, TInput, TArgInput, TOutput, TArgOutput,
                                                       TFromState, TArgFromState, TToState, TArgToState,
                                                       TPivot, TArgPivot>())
        : TBase(std::move(other.Input), std::move(other.Output), other.ErrorCode, other.ErrorCount,
                other.FromState, other.ToState),
          Pivot(std::move(other.Pivot)), PivotErrorCode(std::move(other.PivotErrorCode)),
          PivotErrorCount(std::move(other.PivotErrorCount))
    {
    }

    /// @brief Constructs a PivotlessRecodeResult with the provided parameters and information,
    /// including whether or not an error was handled.
    /// @param[in] input The input range to store.
    /// @param[in] output The output range to store.
    /// @param[in] fromState The state related to the "From Encoding" that performed the decode half of the
    /// operation.
    /// @param[in] toState The state related to the "To Encoding" that performed the encode half of the
    /// operation.
    /// @param[in] errorCode The error code for the recode operation, taken as the first of either the
    /// encode or decode operation that failed.
    /// @param[in] errorCount Whether or not an error was handled. Some error handlers are corrective (see
    /// Handlers::ReplacementHandler), and so the error code is not enough to determine if the handler was
    /// invoked. This allows the value to be provided directly when constructing this result type.
    /// @param[in] pivot The pivot that was used for the recode operation.
    /// @param[in] pivotErrorCode The error code for the decode step of the recode oepration, if it
    /// failed.
    /// @param[in] pivotErrorCount Whether or not an error was handled during the decode step of the
    /// recode operation. Some error handlers are corrective (see Handlers::ReplacementHandler), and so the
    /// error code is not enough to determine if the handler was invoked. This allows the value to be provided
    /// directly when constructing this result type.
    template <typename TArgInput, typename TArgOutput, typename TArgFromState, typename TArgToState,
              typename TArgPivot>
    constexpr RecodeResult(TArgInput &&input, TArgOutput &&output, TArgFromState &&fromState,
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
    constexpr StatelessRecodeResult<TInput, TOutput>
      SliceToStateless(PivotlessRecodeResult<TInput, TOutput, TFromState, TToState> &&result) noexcept(
        NoThrowConstructible<StatelessRecodeResult<TInput, TOutput>, StatelessRecodeResult<TInput, TOutput>>)
    {
      return std::move(result);
    }

    template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
    constexpr PivotlessRecodeResult<TInput, TOutput, TFromState, TToState>
      SliceToPivotless(RecodeResult<TInput, TOutput, TFromState, TToState, TPivot> &&result) noexcept(
        NoThrowConstructible<PivotlessRecodeResult<TInput, TOutput, TFromState, TToState>,
                             PivotlessRecodeResult<TInput, TOutput, TFromState, TToState>>)
    {
      return std::move(result);
    }

    template <typename TInput, typename TOutput, typename TFromState, typename TToState, typename TPivot>
    constexpr StatelessRecodeResult<TInput, TOutput> SliceToStatelessPivotless(
      RecodeResult<TInput, TOutput, TFromState, TToState, TPivot>
        &&result) noexcept(NoThrowConstructible<StatelessRecodeResult<TInput, TOutput>,
                                                StatelessRecodeResult<TInput, TOutput>>)
    {
      return std::move(result);
    }

    template <typename TInput, typename TOutput, typename TDesiredOutput, typename TFromState,
              typename TToState>
    constexpr auto ReplaceRecodeResultOutputNoState(
      PivotlessRecodeResult<TInput, TOutput, TFromState, TToState> &&result,
      TDesiredOutput
        &&desiredOutput) noexcept(NoThrowConstructible<StatelessRecodeResult<TInput, TOutput>, TInput &&,
                                                       TDesiredOutput, EncodingError, std::size_t>)
    {
      using TResult = StatelessRecodeResult<TInput, remove_cvref_t<TDesiredOutput>>;
      return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput), result.ErrorCode,
                     result.ErrorCount);
    }

    template <typename TInput, typename TOutput, typename TFromState, typename TToState,
              typename TDesiredOutput>
    constexpr auto ReplaceRecodeResultOutput(
      PivotlessRecodeResult<TInput, TOutput, TFromState, TToState> &&result,
      TDesiredOutput
        &&desiredOutput) noexcept(NoThrowConstructible<PivotlessRecodeResult<TInput, TOutput, TFromState,
                                                                             TToState>,
                                                       TInput &&, TDesiredOutput, TFromState &, TToState &,
                                                       EncodingError, std::size_t>)
    {
      using TResult = PivotlessRecodeResult<TInput, remove_cvref_t<TDesiredOutput>, TFromState, TToState>;
      return TResult(std::move(result.Input), std::forward<TDesiredOutput>(desiredOutput), result.FromState,
                     result.ToState, result.ErrorCode, result.ErrorCount);
    }

    template <typename TInputRange, typename _OutputRange, typename TFromState, typename TToState>
    using reconstruct_recode_result_t =
      PivotlessRecodeResult<Krys::Ranges::range_reconstruct_t<TInputRange>,
                            Krys::Ranges::range_reconstruct_t<_OutputRange>, TFromState, TToState>;

    template <typename TInputRange, typename _OutputRange, typename TFromState, typename TToState,
              typename TPivot>
    using reconstruct_pivot_recode_result_t =
      RecodeResult<Krys::Ranges::range_reconstruct_t<TInputRange>,
                   Krys::Ranges::range_reconstruct_t<_OutputRange>, TFromState, TToState,
                   Krys::Ranges::range_reconstruct_t<TPivot>>;
  }
}
