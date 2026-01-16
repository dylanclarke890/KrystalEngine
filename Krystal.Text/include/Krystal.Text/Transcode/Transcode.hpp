#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/Decode.hpp"
#include "Krystal.Text/_detail/IsLossless.hpp"
#include "Krystal.Text/_detail/SpanReconstruct.hpp"
#include "Krystal.Text/_detail/UpdateInput.hpp"
#include "Krystal.Text/Encode/Encode.hpp"
#include "Krystal.Text/Encodings/DefaultEncoding.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Transcode/TranscodeOne.hpp"
#include "Krystal.Text/Transcode/TranscodeResult.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <string>
#include <utility>
#include <vector>

/// These functions convert from a view of input code units into a view of output code units using either the
/// inferred or specified encodings. If no error handler is provided, DefaultHandler is used by default. If no
/// state is provided for the encoding, it will created automatically as needed.
namespace Krys::Text
{
  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A TranscodeResult object that contains references to `fromState` and `toState`.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto BasicTranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                       TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                       TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                       TToState &toState, TPivot &&pivot)
  {
    static_assert(::Krys::Text::TranscodeLosslessOrDeliberate<TFromEncoding, TToEncoding, TFromErrorHandler,
                                                              TToErrorHandler>,
                  KRYS_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE);

    using TInitialInput = ::Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
    using TInitialOutput = ::Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
    using TResult = decltype(::Krys::Text::TranscodeOneIntoRaw(
      std::declval<TInitialInput>(), fromEncoding, std::declval<TInitialOutput>(), toEncoding,
      fromErrorHandler, toErrorHandler, fromState, toState, pivot));

    using TWorkingInput = decltype(std::declval<TResult>().Input);
    using TWorkingOutput = decltype(std::declval<TResult>().Output);

    TWorkingInput workingInput(std::forward<TInput>(input));
    TWorkingOutput workingOutput(std::forward<TOutput>(output));

    std::size_t errorCount = 0;
    std::size_t pivotErrorCount = 0;
    for (;;)
    {
      auto transcodeResult = ::Krys::Text::TranscodeOneIntoRaw(
        std::move(workingInput), fromEncoding, std::move(workingOutput), toEncoding, fromErrorHandler,
        toErrorHandler, fromState, toState, pivot);
      workingInput = std::move(transcodeResult.Input);
      workingOutput = std::move(transcodeResult.Output);

      errorCount += transcodeResult.ErrorCount;
      pivotErrorCount += transcodeResult.PivotErrorCount;

      if (transcodeResult.ErrorCode != EncodingError::OK)
      {
        return TResult(std::move(workingInput), std::move(workingOutput), fromState, toState,
                       transcodeResult.ErrorCode, transcodeResult.ErrorCount,
                       std::move(transcodeResult.Pivot), transcodeResult.PivotErrorCode,
                       transcodeResult.PivotErrorCount);
      }
      if (::Krys::Ranges::empty(workingInput))
      {
        if (!::Krys::Text::IsStateComplete(fromEncoding, fromState))
        {
          continue;
        }
        if (!::Krys::Text::IsStateComplete(toEncoding, toState))
        {
          continue;
        }
        break;
      }
    }
    return TResult(std::move(workingInput), std::move(workingOutput), fromState, toState, EncodingError::OK,
                   errorCount, std::forward<TPivot>(pivot), EncodingError::OK, pivotErrorCount);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A TranscodeResult object that contains references to `fromState` and `toState`.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState,
                                  TPivot &&pivot)
  {
    if constexpr (::Krys::Text::HasTranscodeText<TInput, TFromEncoding, TOutput, TToEncoding,
                                                 TFromErrorHandler, TToErrorHandler, TFromState, TToState,
                                                 TPivot>)
    {
      return TranscodeText(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                           std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                           std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                           std::forward<TFromErrorHandler>(fromErrorHandler),
                           std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      if constexpr (IsDecodeRedundant<TFromEncoding, TToEncoding>    // cf
                    && IsEncodeRedundant<TFromEncoding, TToEncoding> // cf
                    && IsIgnorableErrorHandler<TFromErrorHandler>    // cf
                    && IsIgnorableErrorHandler<TToErrorHandler>)
      {
        // we can simply copy from the input to the output, no questions asked!
        (void)fromEncoding;
        (void)toEncoding;
        (void)fromErrorHandler;
        (void)toErrorHandler;
        (void)fromState;
        (void)toState;

        auto result = ::Krys::Ranges::Impl::Copy(
          ::Krys::Ranges::cbegin(std::forward<TInput>(input)), ::Krys::Ranges::cend(input),
          ::Krys::Ranges::begin(std::forward<TOutput>(output)), ::Krys::Ranges::end(output));

        using TResult = TranscodeResult<decltype(result.In), decltype(result.Out), TFromState, TToState,
                                        remove_cvref_t<TPivot>>;
        return TResult(std::move(result.In), std::move(result.Out), fromState, toState, EncodingError::OK, 0,
                       std::forward<TPivot>(pivot), EncodingError::OK, 0);
      }
      else
      {
        return ::Krys::Text::BasicTranscodeIntoRaw(
          std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
          std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
          std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
          fromState, toState, pivot);
      }
    }
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @result A PivotlessTranscodeResult object that contains references to `fromState` and `toState`.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState)
  {
    using TCodePoint = code_point_t<TFromEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxCodePoints<TFromEncoding>;

    TCodePoint buffer[MaxBufferSize] {};
    ::Krys::Ranges::subrange<TCodePoint *> pivot(buffer);

    return ::Krys::Text::TranscodeIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    auto toState = ::Krys::Text::CreateEncodeState(toEncoding);
    auto statefulResult = ::Krys::Text::TranscodeIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return ::Krys::Text::Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler)
  {
    auto fromState = ::Krys::Text::CreateDecodeState(fromEncoding);
    return ::Krys::Text::TranscodeIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = ::Krys::Text::Handlers::CopyOrDefault(fromErrorHandler);
    return ::Krys::Text::TranscodeIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto TranscodeIntoRaw(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TFromEncoding =
      default_code_unit_encoding_t<::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>>;

    TFromEncoding fromEncoding {};
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeIntoRaw(std::forward<TInput>(input), fromEncoding,
                                          std::forward<TOutput>(output),
                                          std::forward<TToEncoding>(toEncoding), handler);
  }
}

namespace Krys::Text::detail_transcode
{
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto IntermediateTranscodeToStorage(TInput &&input, TFromEncoding &&fromEncoding, TOutput &output,
                                                TToEncoding &&toEncoding,
                                                TFromErrorHandler &&fromErrorHandler,
                                                TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                                TToState &toState, TPivot &&pivot)
  {
    static_assert(::Krys::Text::TranscodeLosslessOrDeliberate<TFromEncoding, TToEncoding, TFromErrorHandler,
                                                              TToErrorHandler>,
                  KRYS_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE);

    using TCodeUnit = code_unit_t<TToEncoding>;
    using TFromProgressHandler =
      ::Krys::Text::Handlers::ProgressHandler<IsIgnorableErrorHandler<TFromErrorHandler>, TFromEncoding>;
    using TToProgressHandler =
      ::Krys::Text::Handlers::ProgressHandler<IsIgnorableErrorHandler<TToErrorHandler>, TToEncoding>;

    using TInitialInput = ::Krys::Ranges::csubrange_for_t<TInput>;
    using TIntermediateOutput = ::Krys::Ranges::subrange<TCodeUnit *>;
    using TTranscodeResult = decltype(::Krys::Text::TranscodeIntoRaw(
      std::declval<TInitialInput>(), fromEncoding, std::declval<TIntermediateOutput>(), toEncoding,
      fromErrorHandler, toErrorHandler, fromState, toState, pivot));
    using TWorkingInput = decltype(std::declval<TTranscodeResult>().Input);

    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxTranscodeCodeUnits<TFromEncoding, TToEncoding>;
    TCodeUnit translationBuffer[MaxBufferSize] {};

    TFromProgressHandler fromProgressHandler {};
    TToProgressHandler toProgressHandler {};
    TWorkingInput workingInput(::Krys::Ranges::cbegin(input), ::Krys::Ranges::cend(input));

    std::size_t errorCount = 0;
    std::size_t pivotErrorCount = 0;
    for (;;)
    {
      fromProgressHandler.Clear();
      toProgressHandler.Clear();

      TIntermediateOutput intermediateOutput(translationBuffer, MaxBufferSize);
      auto result =
        ::Krys::Text::TranscodeIntoRaw(std::move(workingInput), fromEncoding, intermediateOutput, toEncoding,
                                       fromProgressHandler, toProgressHandler, fromState, toState, pivot);

      std::size_t writtenCount = static_cast<std::size_t>(result.Output.data() - intermediateOutput.data());
      TIntermediateOutput intermediateWritten(intermediateOutput.begin(),
                                              intermediateOutput.begin() + writtenCount);

      ::Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateWritten);
      if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
      {
        if (toProgressHandler.CodeUnitsProgressSize() != 0)
        {
          ::Krys::Ranges::Impl::ContainerInsertBulk(output, toProgressHandler.CodeUnitsProgress());
          errorCount += result.ErrorCount;
          pivotErrorCount += result.PivotErrorCount;
          workingInput = ::Krys::Text::detail::UpdateConstInput<TWorkingInput>(std::move(result.Input));
          continue;
        }
        else if (result.PivotErrorCode == EncodingError::OK)
        {
          // If this occured, we need to record the original pivot position, and then try to
          // re-serialize with enough space all over again to avoid issues.
          std::size_t pivotRemnantCount =
            static_cast<std::size_t>(::Krys::Ranges::size(pivot) - ::Krys::Ranges::size(result.Pivot));
          auto pivotRemnant =
            ::Krys::Ranges::reconstruct(std::in_place_type<TPivot>, ::Krys::Ranges::cbegin(pivot),
                                        ::Krys::Ranges::cbegin(pivot) + pivotRemnantCount);
          auto pivotResult = ::Krys::Text::EncodeIntoRaw(pivotRemnant, toEncoding, intermediateOutput,
                                                         toErrorHandler, toState);
          std::size_t intermediateWrittenCount =
            static_cast<std::size_t>(pivotResult.Output.data() - intermediateOutput.data());
          TIntermediateOutput pivotIntermediateOutput(intermediateOutput.begin(),
                                                      intermediateOutput.begin() + intermediateWrittenCount);
          ::Krys::Ranges::Impl::ContainerInsertBulk(output, pivotIntermediateOutput);
          errorCount += pivotResult.ErrorCount;
          pivotErrorCount += result.PivotErrorCount;
          if (pivotResult.ErrorCode == EncodingError::OK)
          {
            workingInput = ::Krys::Text::detail::UpdateConstInput<TWorkingInput>(std::move(result.Input));
            continue;
          }
        }
        else
        {
          // it's okay, just loop around, we've got S P A C E for more
          workingInput = ::Krys::Text::detail::UpdateConstInput<TWorkingInput>(std::move(result.Input));
          continue;
        }
      }
      if (result.ErrorCode != EncodingError::OK)
      {
        using TErrorDecodeResult = DecodeResult<decltype(result.Input), decltype(result.Pivot), TFromState>;
        using TErrorEncodeResult = EncodeResult<decltype(result.Pivot), decltype(result.Output), TToState>;
        if (result.PivotErrorCode != EncodingError::OK)
        {
          // need to call the error handler and then propagate it.
          auto errorResult = ::Krys::Text::PropagateTranscodeDecodeError<TTranscodeResult>(
            intermediateOutput, fromEncoding, toEncoding,
            TErrorDecodeResult(std::move(result.Input), std::move(result.Pivot), fromState,
                               result.PivotErrorCode, result.PivotErrorCount),
            fromErrorHandler, toErrorHandler, toState, fromProgressHandler.CodeUnitsProgress(),
            fromProgressHandler.CodePointsProgress(), toProgressHandler.CodePointsProgress(),
            toProgressHandler.CodeUnitsProgress());
          std::size_t errorWrittenCount =
            static_cast<std::size_t>(errorResult.Output.data() - intermediateOutput.data());
          TIntermediateOutput errorIntermediateOutput(intermediateOutput.begin(),
                                                      intermediateOutput.begin() + errorWrittenCount);
          ::Krys::Ranges::Impl::ContainerInsertBulk(output, errorIntermediateOutput);
          errorCount += errorResult.ErrorCount;
          pivotErrorCount += errorResult.PivotErrorCount;
          if (errorResult.ErrorCode != EncodingError::OK)
          {
            return TTranscodeResult(std::move(errorResult.Input), std::move(errorResult.Output),
                                    errorResult.FromState, errorResult.ToState, errorResult.ErrorCode,
                                    errorCount, std::move(errorResult.Pivot), errorResult.PivotErrorCode,
                                    pivotErrorCount);
          }
          workingInput = ::Krys::Text::detail::UpdateConstInput<TWorkingInput>(std::move(errorResult.Input));
        }
        else
        {
          // just need to call the second edge of the failure
          auto errorResult = ::Krys::Text::PropagateTranscodeEncodeError<TTranscodeResult>(
            toEncoding,
            TErrorDecodeResult(std::move(result.Input), std::move(result.Pivot), fromState,
                               result.PivotErrorCode, result.PivotErrorCount),
            TErrorEncodeResult(std::move(result.Pivot), intermediateOutput, toState, result.ErrorCode,
                               result.ErrorCount),
            toErrorHandler, toProgressHandler.CodePointsProgress(), toProgressHandler.CodeUnitsProgress());
          std::size_t errorWrittenCount = errorResult.Output.data() - intermediateOutput.data();
          TIntermediateOutput errorIntermediateOutput(intermediateOutput.begin(),
                                                      intermediateOutput.begin() + errorWrittenCount);
          ::Krys::Ranges::Impl::ContainerInsertBulk(output, errorIntermediateOutput);
          errorCount += errorResult.ErrorCount;
          pivotErrorCount += errorResult.PivotErrorCount;
          if (errorResult.ErrorCode != EncodingError::OK)
          {
            return TTranscodeResult(std::move(errorResult.Input), std::move(errorResult.Output),
                                    errorResult.FromState, errorResult.ToState, errorResult.ErrorCode,
                                    errorCount, std::move(errorResult.Pivot), errorResult.PivotErrorCode,
                                    pivotErrorCount);
          }
          workingInput = ::Krys::Text::detail::UpdateConstInput<TWorkingInput>(std::move(errorResult.Input));
        }
        continue;
      }
      if (::Krys::Ranges::empty(result.Input) && ::Krys::Text::IsStateComplete(fromEncoding, fromState)
          && ::Krys::Text::IsStateComplete(toEncoding, toState))
      {
        errorCount += result.ErrorCount;
        pivotErrorCount += result.PivotErrorCount;
        return TTranscodeResult(std::move(result.Input), std::move(result.Output), result.FromState,
                                result.ToState, result.ErrorCode, errorCount, std::move(result.Pivot),
                                result.PivotErrorCode, pivotErrorCount);
      }
      workingInput = ::Krys::Text::detail::UpdateConstInput<TWorkingInput>(std::move(result.Input));
    }
  }

  template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput, typename TFromEncoding,
            typename TToEncoding, typename TFromErrorHandler, typename TToErrorHandler, typename TFromState,
            typename TToState, typename TPivot>
  constexpr auto TranscodeDispatch(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                   TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                   TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    TOutputContainer output {};
    if constexpr (::Krys::Ranges::HasSizeADL<TInput>)
    {
      using TSize = decltype(::Krys::Ranges::size(input));
      if constexpr (::Krys::Ranges::has_reserve_with_size<TOutputContainer, TSize>)
      {
        output.reserve(Krys::Ranges::size(input));
      }
    }

    auto statefulResult = ::Krys::Text::detail_transcode::IntermediateTranscodeToStorage(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), output,
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);

    if constexpr (OutputOnly)
    {
      (void)statefulResult;
      return output;
    }
    else if constexpr (NoState)
    {
      return ::Krys::Text::Impl::ReplaceTranscodeResultOutputNoState(std::move(statefulResult),
                                                                     std::move(output));
    }
    else
    {
      return ::Krys::Text::Impl::ReplaceTranscodeResultOutput(std::move(statefulResult), std::move(output));
    }
  }
}

namespace Krys::Text
{
  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A TranscodeResult object that contains references to `fromState` and `toState`.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState,
                               TPivot &&pivot)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = ::Krys::Text::TranscodeIntoRaw(
      std::move(reconstructedInput), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    using TReconstructedResultInput = ::Krys::Text::detail::span_reconstruct_t<TInput, decltype(result.Input) &&>;
    using TReconstructedResultOutput = ::Krys::Text::detail::span_reconstruct_mutable_t<TOutput, decltype(result.Output) &&>;
    using TResult = TranscodeResult<TReconstructedResultInput, TReconstructedResultOutput, TFromState,
                                    TToState, decltype(result.Pivot)>;
    return TResult(::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
                   ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)),
                   result.FromState, result.ToState, result.ErrorCode, result.ErrorCount,
                   std::move(result.Pivot), result.PivotErrorCode, result.PivotErrorCount);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @result A PivotlessTranscodeResult object that contains references to `fromState` and `toState`.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState)
  {
    using TCodePoint = code_point_t<TFromEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxCodePoints<TFromEncoding>;

    TCodePoint buffer[MaxBufferSize] {};
    ::Krys::Ranges::subrange<TCodePoint *> pivot(buffer);

    return ::Krys::Text::TranscodeInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    auto toState = ::Krys::Text::CreateEncodeState(toEncoding);
    auto statefulResult = ::Krys::Text::TranscodeInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return ::Krys::Text::Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler)
  {
    auto fromState = ::Krys::Text::CreateDecodeState(fromEncoding);
    return ::Krys::Text::TranscodeInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                       std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                       std::forward<TFromErrorHandler>(fromErrorHandler),
                                       std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = ::Krys::Text::Handlers::CopyOrDefault(fromErrorHandler);
    return ::Krys::Text::TranscodeInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                       std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                       std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                       std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                       handler);
  }

  /// @brief Converts code units from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto TranscodeInto(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>;
    using TFromEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TFromEncoding fromEncoding {};
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeInto(std::forward<TInput>(input), fromEncoding,
                                       std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                       handler);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A TranscodeResult object that has an output of type `TOutputContainer` and contains references
  /// to `fromState` and `toState`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TCodeUnit = code_unit_t<TToEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodeUnit>;
      return ::Krys::Text::detail_transcode::TranscodeDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TOutput = conditional_t<IsVoidContainer, std::vector<TCodeUnit>, TOutputContainer>;
      return ::Krys::Text::detail_transcode::TranscodeDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @result A PivotlessTranscodeResult object that has an output of type `TOutputContainer` and contains
  /// references to `fromState` and `toState`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState, TToState &toState)
  {
    using TCodePoint = code_point_t<TFromEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxCodePoints<TFromEncoding>;

    TCodePoint buffer[MaxBufferSize] {};
    ::Krys::Ranges::subrange<TCodePoint *> pivot(buffer);

    return ::Krys::Text::TranscodeTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState)
  {
    auto toState = ::Krys::Text::CreateEncodeState(toEncoding);
    return ::Krys::Text::TranscodeTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    auto fromState = Krys::Text::CreateDecodeState(fromEncoding);
    return ::Krys::Text::TranscodeTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = ::Krys::Text::Handlers::CopyOrDefault(fromErrorHandler);
    return ::Krys::Text::TranscodeTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeTo<TOutputContainer>(std::forward<TInput>(input),
                                                       std::forward<TFromEncoding>(fromEncoding),
                                                       std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TToEncoding>
  constexpr auto TranscodeTo(TInput &&input, TToEncoding &&toEncoding)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>;
    using TFromEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    ::Krys::Text::Handlers::DefaultHandler handler {};
    TFromEncoding fromEncoding {};
    return ::Krys::Text::TranscodeTo<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                       std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TCodeUnit = code_unit_t<TToEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = std::basic_string<TCodeUnit>;
      return ::Krys::Text::detail_transcode::TranscodeDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TOutput = conditional_t<IsVoidContainer, std::vector<TCodeUnit>, TOutputContainer>;
      return ::Krys::Text::detail_transcode::TranscodeDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @result A object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState, TToState &toState)
  {
    using TCodePoint = code_point_t<TFromEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxCodePoints<TFromEncoding>;

    TCodePoint buffer[MaxBufferSize] {};
    ::Krys::Ranges::subrange<TCodePoint *> pivot(buffer);

    return Krys::Text::Transcode<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @result A object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState)
  {
    auto toState = ::Krys::Text::CreateEncodeState(toEncoding);
    return ::Krys::Text::Transcode<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @result A object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    auto fromState = ::Krys::Text::CreateDecodeState(fromEncoding);
    return ::Krys::Text::Transcode<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result A object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = ::Krys::Text::Handlers::CopyOrDefault(fromErrorHandler);
    return ::Krys::Text::Transcode<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::Transcode<TOutputContainer>(std::forward<TInput>(input),
                                                     std::forward<TFromEncoding>(fromEncoding),
                                                     std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts code units from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TToEncoding>
  constexpr auto Transcode(TInput &&input, TToEncoding &&toEncoding)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>;
    using TFromEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TFromEncoding fromEncoding {};
    return ::Krys::Text::Transcode<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                     std::forward<TToEncoding>(toEncoding));
  }
}
