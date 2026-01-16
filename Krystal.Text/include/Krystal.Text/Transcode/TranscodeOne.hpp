#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Ranges/SaveRange.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Decode/DecodeOne.hpp"
#include "Krystal.Text/_detail/IsLossless.hpp"
#include "Krystal.Text/Encodings/DefaultEncoding.hpp"
#include "Krystal.Text/Handlers/DefaultHandler.hpp"
#include "Krystal.Text/Handlers/ProgressHandler.hpp"
#include "Krystal.Text/_detail/SpanReconstruct.hpp"
#include "Krystal.Text/InlineContainers/InlineBasicString.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/Transcode/PropagateTranscodeError.hpp"
#include "Krystal.Text/Transcode/TranscodeResult.hpp"
#include "Krystal.Text/TypeTraits.hpp"

/// These functions convert from a view of input code units into a view of output code units using either the
/// inferred or specified encodings. If no error handler is provided, DefaultHandler is used by default. If no
/// state is provided for the encoding, it will created automatically as needed.
namespace Krys::Text
{
  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
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
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto BasicTranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                          TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                          TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                          TToState &toState, TPivot &&pivot)
  {
    static_assert(::Krys::Text::TranscodeLosslessOrDeliberate<TFromEncoding, TToEncoding, TFromErrorHandler,
                                                              TToErrorHandler>,
                  KRYS_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE);

    using TCvFromEncoding = remove_ref_t<TFromEncoding>;
    using TCvToEncoding = remove_ref_t<TToEncoding>;
    using TCVFromErrorHandler = remove_ref_t<TFromErrorHandler>;
    using TCVToErrorHandler = remove_ref_t<TToErrorHandler>;

    constexpr bool IsFromProgressHandler = ::Krys::Text::Handlers::IsProgressHandler<TFromErrorHandler>;
    constexpr bool IsToProgressHandler = ::Krys::Text::Handlers::IsProgressHandler<TToErrorHandler>;
    constexpr bool FromAssumeValid = IsIgnorableErrorHandler<TFromErrorHandler>;
    constexpr bool ToAssumeValid = IsIgnorableErrorHandler<TToErrorHandler>;

    using TFromProgressHandler =
      conditional_t<IsFromProgressHandler, TCVFromErrorHandler &,
                    ::Krys::Text::Handlers::ProgressHandler<FromAssumeValid, TCvFromEncoding>>;
    using TToProgressHandler =
      conditional_t<IsToProgressHandler, TCVToErrorHandler &,
                    ::Krys::Text::Handlers::ProgressHandler<ToAssumeValid, TCvToEncoding>>;
    using TFromProgressHandlerRef =
      conditional_t<IsFromProgressHandler, TCVFromErrorHandler &, TFromProgressHandler &>;
    using TToProgressHandlerRef =
      conditional_t<IsToProgressHandler, TCVToErrorHandler &, TToProgressHandler &>;
    using TWorkingOutput = ::Krys::Ranges::subrange_for_t<TOutput>;
    using TIntermediateResult =
      decltype(std::forward<TFromEncoding>(fromEncoding)
                 .DecodeOne(std::forward<TInput>(input), std::forward<TPivot>(pivot),
                            std::declval<TFromProgressHandlerRef>(), fromState));
    using TResultPivot = decltype(std::declval<TIntermediateResult>().Output);
    using TEndResult = decltype(std::forward<TToEncoding>(toEncoding)
                                  .EncodeOne(std::declval<TResultPivot>(), std::declval<TWorkingOutput>(),
                                             std::declval<TToProgressHandlerRef>(), toState));
    using TResultInput = decltype(std::declval<TIntermediateResult>().Input);
    using TResultOutput = decltype(std::declval<TEndResult>().Output);
    using TResult = TranscodeResult<TResultInput, TResultOutput, TFromState, TToState, TResultPivot>;

    auto savedInput = ::Krys::Ranges::SaveRange(input);
    TResultOutput workingOutput(std::forward<TOutput>(output));
    TFromProgressHandler fromIntermediateHandler(fromErrorHandler);
    TToProgressHandler toIntermediateHandler(toErrorHandler);
    auto intermediateResult = std::forward<TFromEncoding>(fromEncoding)
                                .DecodeOne(std::forward<TInput>(input), std::forward<TPivot>(pivot),
                                           fromIntermediateHandler, fromState);
    if (intermediateResult.ErrorCode != EncodingError::OK)
    {
      if constexpr (IsFromProgressHandler && IsToProgressHandler)
      {
        return TResult(std::move(intermediateResult.Input), std::move(workingOutput),
                       intermediateResult.State, toState, intermediateResult.ErrorCode,
                       intermediateResult.ErrorCount, std::move(intermediateResult.Output),
                       intermediateResult.ErrorCode, intermediateResult.ErrorCount);
      }
      else
      {
        return ::Krys::Text::PropagateTranscodeDecodeErrorWith<TResult>(
          ::Krys::Ranges::RestoreRange(std::move(savedInput), std::move(intermediateResult.Input)),
          std::move(workingOutput), fromEncoding, toEncoding, std::move(intermediateResult), fromErrorHandler,
          toErrorHandler, toState, fromIntermediateHandler.CodeUnitsProgress(),
          fromIntermediateHandler.CodePointsProgress(), toIntermediateHandler.CodePointsProgress(),
          toIntermediateHandler.CodeUnitsProgress());
      }
    }
    const std::size_t pivotSize = ::Krys::Ranges::size(pivot);
    std::size_t intermediateWritten = pivotSize - ::Krys::Ranges::size(intermediateResult.Output);
    std::size_t intermediateStart = 0;
    std::size_t errorCount = intermediateResult.ErrorCount;
    for (;;)
    {
      ::Krys::Ranges::subrange<decltype(::Krys::Ranges::cbegin(pivot))> workingIntermediate(
        ::Krys::Ranges::cbegin(pivot) + intermediateStart,
        ::Krys::Ranges::cbegin(pivot) + intermediateWritten);
      std::size_t intermediateSize = Krys::Ranges::size(workingIntermediate);
      auto endResult = std::forward<TToEncoding>(toEncoding)
                         .EncodeOne(std::move(workingIntermediate), std::move(workingOutput),
                                    toIntermediateHandler, toState);
      errorCount += endResult.ErrorCount;
      if (endResult.ErrorCode != EncodingError::OK)
      {
        if constexpr (IsFromProgressHandler && IsToProgressHandler)
        {
          return TResult(std::move(intermediateResult.Input), std::move(endResult.Output),
                         intermediateResult.State, endResult.State, endResult.ErrorCode, errorCount,
                         std::move(intermediateResult.Output), intermediateResult.ErrorCode,
                         intermediateResult.ErrorCount);
        }
        else
        {
          return ::Krys::Text::PropagateTranscodeEncodeErrorWith<TResult>(
            ::Krys::Ranges::RestoreRange(std::move(savedInput), std::move(intermediateResult.Input)),
            toEncoding, std::move(intermediateResult), std::move(endResult), toErrorHandler,
            toIntermediateHandler.CodePointsProgress(), toIntermediateHandler.CodeUnitsProgress());
        }
      }
      if (::Krys::Ranges::empty(endResult.Input))
      {
        if constexpr (IsFromProgressHandler && IsToProgressHandler)
        {
          return TResult(std::move(intermediateResult.Input), std::move(endResult.Output),
                         intermediateResult.State, endResult.State, endResult.ErrorCode, errorCount,
                         std::move(intermediateResult.Output), intermediateResult.ErrorCode,
                         intermediateResult.ErrorCount);
        }
        else
        {
          return TResult(std::move(intermediateResult.Input), std::move(endResult.Output),
                         intermediateResult.State, endResult.State, endResult.ErrorCode, errorCount,
                         std::move(intermediateResult.Output), intermediateResult.ErrorCode,
                         intermediateResult.ErrorCount);
        }
      }
      intermediateStart = intermediateSize - ::Krys::Ranges::size(endResult.Input);
      workingOutput = std::move(endResult.Output);
    }
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
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
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                     TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                     TToState &toState, TPivot &&pivot)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUOutput = remove_cvref_t<TOutput>;
    using TUPivot = remove_cvref_t<TPivot>;
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
      (void)pivot;

      auto result = ::Krys::Ranges::Impl::Copy(
        ::Krys::Ranges::cbegin(std::forward<TInput>(input)), ::Krys::Ranges::cend(input),
        ::Krys::Ranges::begin(std::forward<TOutput>(output)), ::Krys::Ranges::end(output));
      using TResult = ::Krys::Text::Impl::reconstruct_pivot_transcode_result_t<TUInput, TUOutput, TFromState,
                                                                               TToState, TUPivot>;
      return TResult(::Krys::Ranges::reconstruct(std::in_place_type<TUInput>, std::move(result.in)),
                     ::Krys::Ranges::reconstruct(std::in_place_type<TUOutput>, std::move(result.out)),
                     fromState, toState, EncodingError::OK, 0uz, std::forward<TPivot>(pivot),
                     EncodingError::OK, 0);
    }
    else
    {
      return ::Krys::Text::BasicTranscodeOneIntoRaw(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, std::forward<TPivot>(pivot));
    }
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
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
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                     TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                     TToState &toState)
  {
    using TCodePoint = code_point_t<TFromEncoding>;

    TCodePoint buffer[::Krys::Text::MaxDecodeCodePoints<TFromEncoding>] {};
    Span<TCodePoint> pivot(buffer);

    return Krys::Text::TranscodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                     TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    auto toState = ::Krys::Text::CreateEncodeState(toEncoding);
    auto statefulResult = ::Krys::Text::TranscodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return ::Krys::Text::Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                     TToErrorHandler &&toErrorHandler)
  {
    auto fromState = ::Krys::Text::CreateDecodeState(fromEncoding);
    return ::Krys::Text::TranscodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = ::Krys::Text::Handlers::CopyOrDefault(fromErrorHandler);
    return ::Krys::Text::TranscodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TFromEncoding =
      default_code_unit_encoding_t<::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>>;

    TFromEncoding fromEncoding {};
    ::Krys::Text::Handlers::DefaultHandler handler {};

    return ::Krys::Text::TranscodeOneIntoRaw(std::forward<TInput>(input), fromEncoding,
                                             std::forward<TOutput>(output),
                                             std::forward<TToEncoding>(toEncoding), handler);
  }
}

namespace Krys::Text::detail_transcode
{
  template <typename TOutputContainer, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto IntermediateTranscodeOneToStorage(TInput &&input, TFromEncoding &&fromEncoding,
                                                   TOutputContainer &output, TToEncoding &&toEncoding,
                                                   TFromErrorHandler &&fromErrorHandler,
                                                   TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                                   TToState &toState, TPivot &&pivot)
  {
    static_assert(::Krys::Text::TranscodeLosslessOrDeliberate<TFromEncoding, TToEncoding, TFromErrorHandler,
                                                              TToErrorHandler>,
                  KRYS_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE);

    using TCodeUnit = code_unit_t<TToEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxCodeUnits<TToEncoding> * 2;

    Array<TCodeUnit, MaxBufferSize> translationBuffer {};
    Span<TCodeUnit> initialOutput(translationBuffer);

    auto result = ::Krys::Text::TranscodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::move(initialOutput),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);

    ::Krys::Ranges::Impl::ContainerInsertBulk(output, Span(translationBuffer.data(), result.Output.data()));
    return result;
  }

  template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput, typename TFromEncoding,
            typename TToEncoding, typename TFromErrorHandler, typename TToErrorHandler, typename TFromState,
            typename TToState, typename TPivot>
  constexpr auto TranscodeOneDispatch(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                      TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                      TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    TOutputContainer output {};
    if constexpr (::Krys::Ranges::HasSizeADL<TInput>)
    {
      using TSize = decltype(::Krys::Ranges::size(input));
      if constexpr (::Krys::Ranges::has_reserve_with_size<TOutputContainer, TSize>)
      {
        output.reserve(::Krys::Ranges::size(input));
      }
    }

    KRYS_MAYBE_UNUSED auto statefulResult = ::Krys::Text::detail_transcode::IntermediateTranscodeOneToStorage(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), output,
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);

    if constexpr (OutputOnly)
    {
      (void)statefulResult;
      return output;
    }
    else
    {
      if constexpr (NoState)
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
}

namespace Krys::Text
{
  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
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
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto BasicTranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                       TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                       TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                       TToState &toState, TPivot &&pivot)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = ::Krys::Text::BasicTranscodeOneIntoRaw(
      std::move(reconstructedInput), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);

    return PivotlessTranscodeResult(
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
      ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.FromState,
      result.ToState, result.ErrorCode, result.ErrorCount);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
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
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState,
                                  TPivot &&pivot)
  {
    auto reconstructedInput = ::Krys::Text::detail::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = ::Krys::Text::TranscodeOneIntoRaw(
      std::move(reconstructedInput), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);

    return PivotlessTranscodeResult(
      ::Krys::Text::detail::SpanReconstruct<TInput>(std::move(result.Input)),
      ::Krys::Text::detail::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.FromState,
      result.ToState, result.ErrorCode, result.ErrorCount);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @result A TranscodeResult object that contains references to `fromState` and `toState`.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState)
  {
    using TCodePoint = code_point_t<TFromEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxCodePoints<TFromEncoding> * 2;

    TCodePoint buffer[MaxBufferSize] {};
    Span<TCodePoint> pivot(buffer);

    return Krys::Text::TranscodeOneInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
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
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    auto toState = ::Krys::Text::CreateEncodeState(toEncoding);
    auto statefulResult = ::Krys::Text::TranscodeOneInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return ::Krys::Text::Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler)
  {
    auto fromState = ::Krys::Text::CreateDecodeState(fromEncoding);
    return ::Krys::Text::TranscodeOneInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = ::Krys::Text::Handlers::CopyOrDefault(fromErrorHandler);

    return ::Krys::Text::TranscodeOneInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeOneInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in the `output` view.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @result A StatelessTranscodeResult object.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto TranscodeOneInto(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TFromEncoding =
      default_code_unit_encoding_t<::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>>;

    TFromEncoding fromEncoding {};
    ::Krys::Text::Handlers::DefaultHandler handler {};

    return ::Krys::Text::TranscodeOneInto(std::forward<TInput>(input), fromEncoding,
                                          std::forward<TOutput>(output),
                                          std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
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
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TCodeUnit = code_unit_t<TToEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxTranscodeCodeUnits<TFromEncoding, TToEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = InlineBasicString<TCodeUnit, MaxBufferSize>;
      return ::Krys::Text::detail_transcode::TranscodeOneDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TOutput =
        conditional_t<!IsVoidContainer, TOutputContainer, InlineVector<TCodeUnit, MaxBufferSize>>;
      return ::Krys::Text::detail_transcode::TranscodeOneDispatch<false, false, TOutput>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @result A TranscodeResult object that has an output of type `TOutputContainer` and contains references
  /// to `fromState` and `toState`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                TFromState &fromState, TToState &toState)
  {
    using TCodePoint = code_point_t<TFromEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxCodePoints<TFromEncoding> * 2;

    TCodePoint buffer[MaxBufferSize] {};
    Span<TCodePoint> pivot(buffer);
    return Krys::Text::TranscodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
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
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                TFromState &fromState)
  {
    auto toState = ::Krys::Text::CreateEncodeState(toEncoding);
    return ::Krys::Text::TranscodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    auto fromState = ::Krys::Text::CreateDecodeState(fromEncoding);
    return ::Krys::Text::TranscodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = ::Krys::Text::Handlers::CopyOrDefault(fromErrorHandler);
    return ::Krys::Text::TranscodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeOneTo<TOutputContainer>(std::forward<TInput>(input),
                                                          std::forward<TFromEncoding>(fromEncoding),
                                                          std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] output An output view to write code units to.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result A StatelessTranscodeResult object that has an output of type `TOutputContainer`.
  template <typename TOutputContainer, typename TInput, typename TToEncoding>
  constexpr auto TranscodeOneTo(TInput &&input, TToEncoding &&toEncoding)
  {
    using TCodeUnit = ::Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>;
    using TFromEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TFromEncoding fromEncoding {};
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeOneTo<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                          std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
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
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                              TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TCodeUnit = code_unit_t<TToEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxTranscodeCodeUnits<TFromEncoding, TToEncoding>;

    constexpr bool IsVoidContainer = Void<remove_cvref_t<TOutputContainer>>;
    if constexpr ((IsCharTraitable<TCodeUnit> || IsUnicodeCodePoint<TCodeUnit>) && IsVoidContainer)
    {
      using TOutput = InlineBasicString<TCodeUnit, MaxBufferSize>;
      return ::Krys::Text::detail_transcode::TranscodeOneDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TOutput =
        conditional_t<!IsVoidContainer, TOutputContainer, InlineVector<TCodeUnit, MaxBufferSize>>;
      return ::Krys::Text::detail_transcode::TranscodeOneDispatch<true, false, TOutput>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding`'s encode step.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                              TFromState &fromState, TToState &toState)
  {
    using TCodePoint = code_point_t<TFromEncoding>;
    constexpr std::size_t MaxBufferSize = ::Krys::Text::MaxCodePoints<TFromEncoding> * 2;

    TCodePoint buffer[MaxBufferSize] {};
    Span<TCodePoint> pivot(buffer);
    return ::Krys::Text::TranscodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding`'s decode step.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                              TFromState &fromState)
  {
    auto toState = ::Krys::Text::CreateEncodeState(toEncoding);
    return ::Krys::Text::TranscodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @param[in] toErrorHandler The handler to use for encoding errors.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    auto fromState = ::Krys::Text::CreateDecodeState(fromEncoding);
    return ::Krys::Text::TranscodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @param[in] fromErrorHandler The handler to use for decoding errors.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = ::Krys::Text::Handlers::CopyOrDefault(fromErrorHandler);
    return ::Krys::Text::TranscodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] fromEncoding The encoding for decoding `input`'s code units into intermediate code points.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeOne<TOutputContainer>(std::forward<TInput>(input),
                                                        std::forward<TFromEncoding>(fromEncoding),
                                                        std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts one code unit from the given `input` view into code units in `TOutputContainer`.
  /// @tparam TOutputContainer The container type to serialize data into.
  /// @param[in] input An input view to read code units from.
  /// @param[in] toEncoding The encoding for encoding intermediate code points into `output` code units.
  /// @result An object of type `TOutputContainer`.
  template <typename TOutputContainer = void, typename TInput, typename TToEncoding>
  constexpr auto TranscodeOne(TInput &&input, TToEncoding &&toEncoding)
  {
    using TCodeUnit = Krys::Ranges::range_value_type_t<remove_cvref_t<TInput>>;
    using TFromEncoding =
      conditional_t<std::is_constant_evaluated(), default_consteval_code_unit_encoding_t<TCodeUnit>,
                    default_code_unit_encoding_t<TCodeUnit>>;

    TFromEncoding fromEncoding {};
    ::Krys::Text::Handlers::DefaultHandler handler {};
    return ::Krys::Text::TranscodeOne<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                        std::forward<TToEncoding>(toEncoding));
  }
}
