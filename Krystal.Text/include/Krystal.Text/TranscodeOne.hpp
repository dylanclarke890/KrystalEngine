#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Ranges/SaveRange.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DecodeOne.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/ErrorHandlers/DefaultHandler.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/ForwardIfMoveOnly.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/Impl/ProgressHandler.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/Impl/UpdateInput.hpp"
#include "Krystal.Text/InlineContainers/InlineString.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/IsRedundant.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/PropagateTranscodeError.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TranscodeResult.hpp"

namespace Krys
{
  /// @brief These functions convert from a view of input code units into a view of output code units
  /// (typically, through an intermediary code point type that is similar between the two) using either the
  /// inferred or specified encodings. If no error handler is provided, the equivalent of the
  /// Handlers::DefaultHandler is used by default, but it is marked as careless. If no associated state is
  /// provided for either the "to" or "from" encodings, one will be created with automatic storage duration
  /// (as a "stack" variable) for the provided encoding.

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. nly one.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A TranscodeResult object that contains references to `fromState` and @p toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to transcode by first decoding the input code
  /// units to code points, then encoding the intermediate code points to the desired, output code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto BasicTranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                          TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                          TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                          TToState &toState, TPivot &&pivot)
  {
    using TCvFromEncoding = remove_ref_t<TFromEncoding>;
    using TCvToEncoding = remove_ref_t<TToEncoding>;
    using TCVFromErrorHandler = remove_ref_t<TFromErrorHandler>;
    using TCVToErrorHandler = remove_ref_t<TToErrorHandler>;
    using TUFromErrorHandler = remove_cvref_t<TFromErrorHandler>;
    using TUToErrorHandler = remove_cvref_t<TToErrorHandler>;
    constexpr bool IsFromProgressHandler = Impl::IsProgressHandler<TUFromErrorHandler>;
    constexpr bool IsToProgressHandler = Impl::IsProgressHandler<TUToErrorHandler>;
    constexpr bool IsBothProgressHandlers = IsFromProgressHandler && IsToProgressHandler;
    constexpr bool FromAssumeValid = IsIgnorableErrorHandler<TUFromErrorHandler>;
    constexpr bool ToAssumeValid = IsIgnorableErrorHandler<TUToErrorHandler>;
    using TFromProgressHandler = conditional_t<IsFromProgressHandler, TCVFromErrorHandler &,
                                               Impl::ProgressHandler<FromAssumeValid, TCvFromEncoding>>;
    using TToProgressHandler = conditional_t<IsToProgressHandler, TCVToErrorHandler &,
                                             Impl::ProgressHandler<ToAssumeValid, TCvToEncoding>>;
    using TFromProgressHandlerRef =
      conditional_t<IsFromProgressHandler, TCVFromErrorHandler &, TFromProgressHandler &>;
    using TToProgressHandlerRef =
      conditional_t<IsToProgressHandler, TCVToErrorHandler &, TToProgressHandler &>;
    using TWorkingOutput = Krys::Ranges::subrange_for_t<TOutput>;
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

    static_assert(
      Impl::DecodeLosslessOrDeliberate<remove_cvref_t<TFromEncoding>, remove_cvref_t<TFromErrorHandler>>,
      ZTD_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE_I_);
    static_assert(
      Impl::EncodeLosslessOrDeliberate<remove_cvref_t<TToEncoding>, remove_cvref_t<TToErrorHandler>>,
      ZTD_TEXT_LOSSY_TRANSCODE_ENCODE_MESSAGE_I_);

    auto savedInput = Krys::Ranges::SaveRange(input);
    TResultOutput workingOutput(std::forward<TOutput>(output));
    TFromProgressHandler fromIntermediateHandler(fromErrorHandler);
    TToProgressHandler toIntermediateHandler(toErrorHandler);
    auto intermediateResult = std::forward<TFromEncoding>(fromEncoding)
                                .DecodeOne(std::forward<TInput>(input), std::forward<TPivot>(pivot),
                                           fromIntermediateHandler, fromState);
    if (intermediateResult.ErrorCode != EncodingError::OK)
    {
      if constexpr (IsBothProgressHandlers)
      {
        return TResult(std::move(intermediateResult.Input), std::move(workingOutput),
                       intermediateResult.State, toState, intermediateResult.ErrorCode,
                       intermediateResult.ErrorCount, std::move(intermediateResult.Output),
                       intermediateResult.ErrorCode, intermediateResult.ErrorCount);
      }
      else
      {
        return Krys::PropagateTranscodeDecodeErrorWith<TResult>(
          Krys::Ranges::RestoreRange(std::move(savedInput), std::move(intermediateResult.Input)),
          std::move(workingOutput), fromEncoding, toEncoding, std::move(intermediateResult), fromErrorHandler,
          toErrorHandler, toState, fromIntermediateHandler.CodeUnitsProgress(),
          fromIntermediateHandler.CodePointsProgress(), toIntermediateHandler.CodePointsProgress(),
          toIntermediateHandler.CodeUnitsProgress());
      }
    }
    const std::size_t pivotSize = Krys::Ranges::size(pivot);
    std::size_t intermediateWritten = pivotSize - Krys::Ranges::size(intermediateResult.Output);
    std::size_t intermediateStart = 0;
    std::size_t errorCount = intermediateResult.ErrorCount;
    for (;;)
    {
      Krys::Ranges::subrange<decltype(Krys::Ranges::cbegin(pivot))> workingIntermediate(
        Krys::Ranges::cbegin(pivot) + intermediateStart, Krys::Ranges::cbegin(pivot) + intermediateWritten);
      std::size_t intermediateSize = Krys::Ranges::size(workingIntermediate);
      auto endResult = std::forward<TToEncoding>(toEncoding)
                         .EncodeOne(std::move(workingIntermediate), std::move(workingOutput),
                                    toIntermediateHandler, toState);
      errorCount += endResult.ErrorCount;
      if (endResult.ErrorCode != EncodingError::OK)
      {
        if constexpr (IsBothProgressHandlers)
        {
          return TResult(std::move(intermediateResult.Input), std::move(endResult.Output),
                         intermediateResult.State, endResult.State, endResult.ErrorCode, errorCount,
                         std::move(intermediateResult.Output), intermediateResult.ErrorCode,
                         intermediateResult.ErrorCount);
        }
        else
        {
          return Krys::PropagateTranscodeEncodeErrorWith<TResult>(
            Krys::Ranges::RestoreRange(std::move(savedInput), std::move(intermediateResult.Input)),
            toEncoding, std::move(intermediateResult), std::move(endResult), toErrorHandler,
            toIntermediateHandler.CodePointsProgress(), toIntermediateHandler.CodeUnitsProgress());
        }
      }
      if (Krys::Ranges::empty(endResult.Input))
      {
        if constexpr (IsBothProgressHandlers)
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
      intermediateStart = intermediateSize - Krys::Ranges::size(endResult.Input);
      workingOutput = std::move(endResult.Output);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A TranscodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to transcode by first
  /// decoding the input code units to code points, then encoding the intermediate code points to the desired,
  /// output code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                     TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                     TToState &toState, TPivot &&pivot)
  {
    if constexpr (Impl::HasTextTranscodeOne<TInput, TOutput, TFromEncoding, TToEncoding, TFromErrorHandler,
                                            TToErrorHandler, TFromState, TToState, TPivot>)
    {
      return TextTranscodeOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                                    std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                    std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                    std::forward<TFromErrorHandler>(fromErrorHandler),
                                    std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TUFromEncoding = remove_cvref_t<TFromEncoding>;
      using TUToEncoding = remove_cvref_t<TToEncoding>;
      using TUFromErrorHandler = remove_cvref_t<TFromErrorHandler>;
      using TUToErrorHandler = remove_cvref_t<TToErrorHandler>;
      using TUInput = remove_cvref_t<TInput>;
      using TUOutput = remove_cvref_t<TOutput>;
      using TUPivot = remove_cvref_t<TPivot>;
      if constexpr (IsDecodeRedundant<TUFromEncoding, TUToEncoding>    // cf
                    && IsEncodeRedundant<TUFromEncoding, TUToEncoding> // cf
                    && IsIgnorableErrorHandler<TUFromErrorHandler>     // cf
                    && IsIgnorableErrorHandler<TUToErrorHandler>)
      {
        // we can simply copy from the input to the output, no questions asked!
        (void)fromEncoding;
        (void)toEncoding;
        (void)fromErrorHandler;
        (void)toErrorHandler;
        (void)fromState;
        (void)toState;
        (void)pivot;
        auto result = Krys::Ranges::Impl::Copy(
          Krys::Ranges::cbegin(std::forward<TInput>(input)), Krys::Ranges::cend(input),
          Krys::Ranges::begin(std::forward<TOutput>(output)), Krys::Ranges::end(output));
        using TResult =
          Impl::reconstruct_pivot_transcode_result_t<TUInput, TUOutput, TFromState, TToState, TUPivot>;
        return TResult(Krys::Ranges::reconstruct(std::in_place_type<TUInput>, std::move(result.in)),
                       Krys::Ranges::reconstruct(std::in_place_type<TUOutput>, std::move(result.out)),
                       fromState, toState, EncodingError::OK, 0, std::forward<TPivot>(pivot),
                       EncodingError::OK, 0);
      }
      else if constexpr (Impl::HasInternalTextTranscodeOne<TInput, TFromEncoding, TOutput, TToEncoding,
                                                           TFromErrorHandler, TToErrorHandler, TFromState,
                                                           TToState, TPivot>)
      {
        return InternalTextTranscodeOne(
          Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
          std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
          std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
          std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
          fromState, toState, std::forward<TPivot>(pivot));
      }
      else
      {
        return Krys::BasicTranscodeOneIntoRaw(
          std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
          std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
          std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
          fromState, toState, std::forward<TPivot>(pivot));
      }
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @result A TranscodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to transcode by first
  /// decoding the input code units to code points, then encoding the intermediate code points to the desired,
  /// output code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                     TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                     TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivot = Span<TCodePoint>;
    constexpr std::size_t intermediateBufferMax = MaxCodePoints<TUFromEncoding> * 2;
    TCodePoint intermediate[intermediateBufferMax] {};
    TPivot pivot(intermediate);
    return Krys::TranscodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @result A StatelessTranscodeResult object.
  /// @remarks This function calls the base reference, the TranscodeOneIntoRaw after creating a
  /// `ToState` from Krys::CreateEncodeState. The result from this function returns a
  /// StatelessTranscodeResult as opposed to a TranscodeResult because the state
  /// information is on the stack, and returning the state in those types by reference will result in
  /// references to memory that has already been cleaned up. If you need access to the state parameters, call
  /// the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                     TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = encode_state_t<TUToEncoding>;

    TToState toState = ::Krys::CreateEncodeState(toEncoding);

    auto statefulResult = Krys::TranscodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @remarks This function creates a decode state `FromState` by calling CreateDecodeState. The
  /// result from this function returns a StatelessTranscodeResult as opposed to a
  /// TranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                     TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = decode_state_t<TUFromEncoding>;

    TFromState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::TranscodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                     std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                     std::forward<TFromErrorHandler>(fromErrorHandler),
                                     std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// TranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Krys::Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);
    return Krys::TranscodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                     std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                     std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// TranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                     TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};
    return Krys::TranscodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                     std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                     handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
  /// marked as careless to pass to the next function overload; and, a `FromEncoding` to interpret the
  /// `input` by checking the `input` 's `value_type.` This matters for lossy conversions that are not
  /// injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// TranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto TranscodeOneIntoRaw(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUFromEncoding = default_code_unit_encoding_t<Krys::Ranges::range_value_type_t<TUInput>>;

    TUFromEncoding fromEncoding {};
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeOneIntoRaw(std::forward<TInput>(input), fromEncoding, std::forward<TOutput>(output),
                                     std::forward<TToEncoding>(toEncoding), handler);
  }

  namespace Impl
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
      using TUFromEncoding = remove_cvref_t<TFromEncoding>;
      using TUToEncoding = remove_cvref_t<TToEncoding>;
      using TUFromErrorHandler = remove_cvref_t<TFromErrorHandler>;
      using TUToErrorHandler = remove_cvref_t<TToErrorHandler>;
      constexpr std::size_t intermediateBufferMax = MaxCodeUnits<TUToEncoding> * 2;
      using TIntermediateValue = code_unit_t<TUToEncoding>;
      using TOutput = Span<TIntermediateValue>;

      static_assert(Impl::DecodeLosslessOrDeliberate<TUFromEncoding, TUFromErrorHandler>,
                    ZTD_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE_I_);
      static_assert(Impl::EncodeLosslessOrDeliberate<TUToEncoding, TUToErrorHandler>,
                    ZTD_TEXT_LOSSY_TRANSCODE_ENCODE_MESSAGE_I_);

      TIntermediateValue intermediateTranslationBuffer[intermediateBufferMax] {};
      TOutput intermediateInitialOutput(intermediateTranslationBuffer);
      auto result =
        Krys::TranscodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::move(intermediateInitialOutput), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler),
                                  std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
      TOutput intermediateOutput(intermediateInitialOutput.data(), result.Output.data());
      Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateOutput);
      return result;
    }

    template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput,
              typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
              typename TToErrorHandler, typename TFromState, typename TToState, typename TPivot>
    constexpr auto TranscodeOneDispatch(TInput &&input, TFromEncoding &&fromEncoding,
                                        TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                        TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                        TToState &toState, TPivot &&pivot)
    {
      TOutputContainer output {};
      if constexpr (Krys::Ranges::has_adl_size<TInput>)
      {
        using TSize = decltype(Krys::Ranges::size(input));
        if constexpr (Krys::Ranges::has_reserve_with_size<TOutputContainer, TSize>)
        {
          TSize outputSizeHint = static_cast<TSize>(Krys::Ranges::size(input));
          output.reserve(outputSizeHint);
        }
      }
      auto statefulResult = Krys::Impl::IntermediateTranscodeOneToStorage(
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
          return Impl::ReplaceTranscodeResultOutputNoState(std::move(statefulResult), std::move(output));
        }
        else
        {
          return Impl::ReplaceTranscodeResultOutput(std::move(statefulResult), std::move(output));
        }
      }
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A TranscodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to transcode by first
  /// decoding the input code units to code points, then encoding the intermediate code points to the desired,
  /// output code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto BasicTranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                       TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                       TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                       TToState &toState, TPivot &&pivot)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::BasicTranscodeOneIntoRaw(
      std::move(reconstructedInput), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
    return PivotlessTranscodeResult<TReconstructedResultInput, TReconstructedResultOutput, TFromState,
                                    TToState>(Impl::SpanReconstruct<TInput>(std::move(result.Input)),
                                              Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)),
                                              result.FromState, result.ToState, result.ErrorCode,
                                              result.ErrorCount);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A TranscodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to transcode by first
  /// decoding the input code units to code points, then encoding the intermediate code points to the desired,
  /// output code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState,
                                  TPivot &&pivot)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::TranscodeOneIntoRaw(
      std::move(reconstructedInput), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, decltype(result.Input)>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, decltype(result.Output)>;
    return PivotlessTranscodeResult<TReconstructedResultInput, TReconstructedResultOutput, TFromState,
                                    TToState>(Impl::SpanReconstruct<TInput>(std::move(result.Input)),
                                              Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)),
                                              result.FromState, result.ToState, result.ErrorCode,
                                              result.ErrorCount);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @result A TranscodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to transcode by first
  /// decoding the input code units to code points, then encoding the intermediate code points to the desired,
  /// output code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivot = Span<TCodePoint>;
    constexpr std::size_t intermediateBufferMax = MaxCodePoints<TUFromEncoding> * 2;

    TCodePoint intermediate[intermediateBufferMax] {};
    TPivot pivot(intermediate);
    return Krys::TranscodeOneInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler),
                                  std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @result A StatelessTranscodeResult object.
  /// @remarks This function calls the base reference, the TranscodeOneInto after creating a
  /// `ToState` from Krys::CreateEncodeState. The result from this function returns a
  /// StatelessTranscodeResult as opposed to a TranscodeResult because the state
  /// information is on the stack, and returning the state in those types by reference will result in
  /// references to memory that has already been cleaned up. If you need access to the state parameters, call
  /// the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = encode_state_t<TUToEncoding>;

    TToState toState = ::Krys::CreateEncodeState(toEncoding);

    auto statefulResult = Krys::TranscodeOneInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @remarks This function creates a decode state `FromState` by calling CreateDecodeState. The
  /// result from this function returns a StatelessTranscodeResult as opposed to a
  /// TranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = decode_state_t<TUFromEncoding>;

    TFromState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::TranscodeOneInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler),
                                  std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// TranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::TranscodeOneInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// TranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto TranscodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeOneInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  ///
  /// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
  /// marked as careless to pass to the next function overload; and, a `FromEncoding` to interpret the
  /// `input` by checking the `input` 's `value_type.` This matters for lossy conversions that are not
  /// injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// TranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto TranscodeOneInto(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUFromEncoding = default_code_unit_encoding_t<Krys::Ranges::range_value_type_t<TUInput>>;

    TUFromEncoding fromEncoding {};
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeOneInto(std::forward<TInput>(input), fromEncoding, std::forward<TOutput>(output),
                                  std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  ///
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  ///
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  ///
  /// @returns A TranscodeResult object that contains references to `fromState` and @p
  /// toState and an `output` parameter that contains the `TOutputContainer` specified. If the container has a
  /// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the
  /// container, to aid with `push_back` / `insert` reallocation pains.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUToEncoding>;
    constexpr std::size_t MinimumIntermediateOutputMax = MaxTranscodeCodeUnits<TUFromEncoding, TUToEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      using TRealOutputContainer = InlineBasicString<TOutputCodeUnit, MinimumIntermediateOutputMax>;
      return Impl::TranscodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodeUnit, MinimumIntermediateOutputMax>,
                      TOutputContainer>;
      return Impl::TranscodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @returns A TranscodeResult object that contains references to `fromState` and @p
  /// toState and an `output` parameter that contains the `TOutputContainer` specified. If the container has a
  /// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the
  /// container, to aid with `push_back` / `insert` reallocation pains.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivot = Span<TCodePoint>;
    constexpr std::size_t intermediateBufferMax = MaxCodePoints<TUFromEncoding> * 2;

    TCodePoint intermediate[intermediateBufferMax] {};
    TPivot pivot(intermediate);
    return Krys::TranscodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  ///
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  ///
  /// @remarks A default state for the encode step of the operation is create using Krys::CreateEncodeState.
  /// The return type is stateless since both states must be passed in. If you want to have access to the
  /// states, create both of them yourself and pass them into a lower-level function that accepts those
  /// parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = encode_state_t<TUToEncoding>;

    TToState toState = ::Krys::CreateEncodeState(toEncoding);

    return Krys::TranscodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A default state for the decode step of the operation is create using CreateDecodeState. The
  /// return type is stateless since both states must be passed in. If you want to have access to the states,
  /// create both of them yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = decode_state_t<TUFromEncoding>;

    TFromState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::TranscodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `to_error_handler` for the encode step of the operation is created using default construction
  /// of a Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states
  /// must be passed in. If you want to have access to the states, create both of them yourself and pass them
  /// into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::TranscodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `from_error_handler` for the encode step of the operation is created using default
  /// construction of a Handlers::DefaultHandler that is marked as careless. The return type is stateless
  /// since both states must be passed in. If you want to have access to the states, create both of them
  /// yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto TranscodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeOneTo<TOutputContainer>(std::forward<TInput>(input),
                                                  std::forward<TFromEncoding>(fromEncoding),
                                                  std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `from_error_handler` for the encode step of the operation is created using default
  /// construction of a Handlers::DefaultHandler that is marked as careless. The return type is stateless
  /// since both states must be passed in. If you want to have access to the states, create both of them
  /// yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer, typename TInput, typename TToEncoding>
  constexpr auto TranscodeOneTo(TInput &&input, TToEncoding &&toEncoding)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      using TUFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      Handlers::DefaultHandler handler {};
      TUFromEncoding fromEncoding {};
      return Krys::TranscodeOneTo<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                    std::forward<TToEncoding>(toEncoding), handler);
    }
    else
    {
      using TUFromEncoding = default_code_unit_encoding_t<TCodeUnit>;
      Handlers::DefaultHandler handler {};
      TUFromEncoding fromEncoding {};
      return Krys::TranscodeOneTo<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                    std::forward<TToEncoding>(toEncoding), handler);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeOneTo or TranscodeOneInto.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                              TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUToEncoding>;
    constexpr std::size_t MinimumIntermediateOutputMax = MaxTranscodeCodeUnits<TUFromEncoding, TUToEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      using TRealOutputContainer = InlineBasicString<TOutputCodeUnit, MinimumIntermediateOutputMax>;
      return Impl::TranscodeOneDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodeUnit, MinimumIntermediateOutputMax>,
                      TOutputContainer>;
      return Impl::TranscodeOneDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeOneTo or TranscodeOneInto.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                              TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivot = Span<TCodePoint>;
    constexpr std::size_t intermediateBufferMax = MaxCodePoints<TUFromEncoding> * 2;

    TCodePoint intermediate[intermediateBufferMax] {};
    TPivot pivot(intermediate);
    return Krys::TranscodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeOneTo or TranscodeOneInto.
  ///
  /// @remarks This function creates an `ToState` for the encoding step of the operation using
  /// Krys::CreateEncodeState.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                              TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = encode_state_t<TUToEncoding>;

    TToState toState = ::Krys::CreateEncodeState(toEncoding);

    return Krys::TranscodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeOneTo or TranscodeOneInto.
  /// @remarks This function creates an `FromState` for the encoding step of the operation using
  /// CreateDecodeState.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = decode_state_t<TUFromEncoding>;

    TFromState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::TranscodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeOneTo or TranscodeOneInto.
  /// @remarks This function creates a `to_error_handler` from a class like Handlers::DefaultHandler, but that
  /// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that
  /// are not injective.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                              TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::TranscodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeOneTo or TranscodeOneInto.
  /// @remarks This function creates a `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto TranscodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeOne<TOutputContainer>(std::forward<TInput>(input),
                                                std::forward<TFromEncoding>(fromEncoding),
                                                std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into
  /// the final code units.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeOneTo or TranscodeOneInto.
  /// @remarks This function creates both: a `from_error_handler` from a class like Handlers::DefaultHandler,
  /// but that is marked as careless since you did not explicitly provide it; and, a `FromEncoding` derived
  /// from the @p "input"'s `value_type.` The careless marking matters for lossy conversions that are not
  /// injective.
  template <typename TOutputContainer = void, typename TInput, typename TToEncoding>
  constexpr auto TranscodeOne(TInput &&input, TToEncoding &&toEncoding)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      using TUFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TUFromEncoding fromEncoding {};
      return Krys::TranscodeOne<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                  std::forward<TToEncoding>(toEncoding));
    }
    else
    {
      using TUFromEncoding = default_code_unit_encoding_t<TCodeUnit>;

      TUFromEncoding fromEncoding {};
      return Krys::TranscodeOne<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                  std::forward<TToEncoding>(toEncoding));
    }
  }
}
