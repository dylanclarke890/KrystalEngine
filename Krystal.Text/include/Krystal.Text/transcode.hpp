#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Impl/InsertBulk.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Decode.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/Encode.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/ForwardIfMoveOnly.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TranscodeOne.hpp"
#include "Krystal.Text/TranscodeResult.hpp"
#include <string>
#include <utility>
#include <vector>

namespace Krys
{
  /// @brief These functions convert from a view of input code units into a view of output code units
  /// (typically, through an intermediary code point type that is similar between the two) using either the
  /// inferred or specified encodings. If no error handler is provided, the equivalent of the
  /// Handlers::DefaultHandler is used by default, but it is marked as careless. If no associated state is
  /// provided for either the "to" or "from" encodings, one will be created with automatic storage duration
  /// (as a "stack" variable) for the provided encoding.

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
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
  /// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to transcode by first decoding the input code
  /// units to code points, then encoding the intermediate code points to the desired, output code units.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto BasicTranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                       TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                       TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                       TToState &toState, TPivot &&pivot)
  {
    using TInitialInput = Krys::Ranges::csubrange_for_t<remove_ref_t<TInput>>;
    using TInitialOutput = Krys::Ranges::subrange_for_t<remove_ref_t<TOutput>>;
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TUFromErrorHandler = remove_cvref_t<TFromErrorHandler>;
    using TUToErrorHandler = remove_cvref_t<TToErrorHandler>;
    using TResult = decltype(Krys::TranscodeOneIntoRaw(std::declval<TInitialInput>(), fromEncoding,
                                                 std::declval<TInitialOutput>(), toEncoding, fromErrorHandler,
                                                 toErrorHandler, fromState, toState, pivot));
    using TWorkingInput = decltype(std::declval<TResult>().Input);
    using TWorkingOutput = decltype(std::declval<TResult>().Output);

    static_assert(Impl::DecodeLosslessOrDeliberate<TUFromEncoding, TUFromErrorHandler>,
                  ZTD_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE_I_);
    static_assert(Impl::EncodeLosslessOrDeliberate<TUToEncoding, TUToErrorHandler>,
                  ZTD_TEXT_LOSSY_TRANSCODE_ENCODE_MESSAGE_I_);

    TWorkingInput workingInput(std::forward<TInput>(input));
    TWorkingOutput workingOutput(std::forward<TOutput>(output));

    std::size_t errorCount = 0;
    std::size_t pivotErrorCount = 0;
    for (;;)
    {
      auto transcodeResult =
        Krys::TranscodeOneIntoRaw(std::move(workingInput), fromEncoding, std::move(workingOutput), toEncoding,
                                  fromErrorHandler, toErrorHandler, fromState, toState, pivot);
      errorCount += transcodeResult.ErrorCount;
      pivotErrorCount += transcodeResult.PivotErrorCount;
      workingInput = std::move(transcodeResult.Input);
      workingOutput = std::move(transcodeResult.Output);
      if (transcodeResult.ErrorCode != EncodingError::OK)
      {
        return TResult(std::move(workingInput), std::move(workingOutput), fromState, toState,
                       transcodeResult.ErrorCode, transcodeResult.ErrorCount,
                       std::move(transcodeResult.Pivot), transcodeResult.PivotErrorCode,
                       transcodeResult.PivotErrorCount);
      }
      if (Krys::Ranges::empty(workingInput))
      {
        if (!Krys::IsStateComplete(fromEncoding, fromState))
        {
          continue;
        }
        if (!Krys::IsStateComplete(toEncoding, toState))
        {
          continue;
        }
        break;
      }
    }
    return TResult(std::move(workingInput), std::move(workingOutput), fromState, toState, EncodingError::OK,
                   errorCount, std::forward<TPivot>(pivot), EncodingError::OK, pivotErrorCount);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
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
  /// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to transcode by first decoding the input code
  /// units to code points, then encoding the intermediate code points to the desired, output code units.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState,
                                  TPivot &&pivot)
  {
    if constexpr (Impl::HasTextTranscode<TInput, TFromEncoding, TOutput, TToEncoding, TFromErrorHandler,
                                         TToErrorHandler, TFromState, TToState, TPivot>)
    {
      return TextTranscode(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
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
        auto result = Krys::Ranges::Impl::Copy(
          Krys::Ranges::cbegin(std::forward<TInput>(input)), Krys::Ranges::cend(input),
          Krys::Ranges::begin(std::forward<TOutput>(output)), Krys::Ranges::end(output));
        using TResult =
          TranscodeResult<decltype(result.in), decltype(result.out), TFromState, TToState, TUPivot>;
        return TResult(std::move(result.in), std::move(result.out), fromState, toState, EncodingError::OK, 0,
                       std::forward<TPivot>(pivot), EncodingError::OK, 0);
      }
      else if constexpr (Impl::HasInternalTextTranscodeOne<TInput, TFromEncoding, TOutput, TToEncoding,
                                                           TFromErrorHandler, TToErrorHandler, TFromState,
                                                           TToState, TPivot>)
      {
        return InternalTextTranscode(
          Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
          std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
          std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
          std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
          fromState, toState, pivot);
      }
      else
      {
        return Krys::BasicTranscodeIntoRaw(
          std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
          std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
          std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
          fromState, toState, pivot);
      }
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @result A PivotlessTranscodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to transcode by first decoding the input code
  /// units to code points, then encoding the intermediate code points to the desired, output code units.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivotRange = Krys::Ranges::subrange<TCodePoint *>;

    constexpr std::size_t pivotBufferBufferMax =
      ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<TUFromEncoding>)
          < MaxCodePoints<TUFromEncoding>
        ? MaxCodePoints<TUFromEncoding>
        : ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<TUFromEncoding>);

    TCodePoint pivotBuffer[pivotBufferBufferMax] {};
    TPivotRange pivot(pivotBuffer);
    return ::TranscodeIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
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
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @result A StatelessTranscodeResult object.
  /// @remarks This function calls the base reference, the TranscodeIntoRaw after creating a `ToState`
  /// from Krys::CreateEncodeState. The result from this function returns a
  /// StatelessTranscodeResult as opposed to a TranscodeResult because the state
  /// information is on the stack, and returning the state in those types by reference will result in
  /// references to memory that has already been cleaned up. If you need access to the state parameters, call
  /// the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = encode_state_t<TUToEncoding>;

    TToState toState = ::Krys::CreateEncodeState(toEncoding);

    auto statefulResult = Krys::TranscodeIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts the code units of the given input view through the
  ///        from encoding to code units of the to encoding into the output
  ///        view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  ///
  /// @remarks This function creates a decode state `FromState` by calling CreateDecodeState. The
  /// result from this function returns a StatelessTranscodeResult as opposed to a
  /// PivotlessTranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = decode_state_t<TUFromEncoding>;

    TFromState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::TranscodeIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler),
                                  std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the
  ///        from encoding to code units of the to encoding into the output
  ///        view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  ///
  /// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// PivotlessTranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::TranscodeIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the
  ///        from encoding to code units of the to encoding into the output
  ///        view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  ///
  /// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// PivotlessTranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto TranscodeIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  handler);
  }

  /// @brief Converts the code units of the given input view through the
  ///        from encoding to code units of the to encoding into the output
  ///        view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  ///
  /// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
  /// marked as careless to pass to the next function overload; and, a `FromEncoding` to interpret the
  /// `input` by checking the `input` 's `value_type.` This matters for lossy conversions that are not
  /// injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// PivotlessTranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto TranscodeIntoRaw(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUFromEncoding = default_code_unit_encoding_t<Krys::Ranges::range_value_type_t<TUInput>>;

    TUFromEncoding fromEncoding {};
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeIntoRaw(std::forward<TInput>(input), fromEncoding, std::forward<TOutput>(output),
                                  std::forward<TToEncoding>(toEncoding), handler);
  }

  namespace Impl
  {
    template <typename TInput, typename TFromEncoding, typename TOutputContainer, typename TToEncoding,
              typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
              typename TPivot>
    constexpr auto IntermediateTranscodeToStorage(TInput &&input, TFromEncoding &&fromEncoding,
                                                  TOutputContainer &output, TToEncoding &&toEncoding,
                                                  TFromErrorHandler &&fromErrorHandler,
                                                  TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                                  TToState &toState, TPivot &&pivot)
    {
      // … Weeeellll. Here we go …
      using TUFromEncoding = remove_cvref_t<TFromEncoding>;
      using TUToEncoding = remove_cvref_t<TToEncoding>;
      using TUFromErrorHandler = remove_cvref_t<TFromErrorHandler>;
      using TUToErrorHandler = remove_cvref_t<TToErrorHandler>;
      using TFromProgressHandler =
        Impl::ProgressHandler<IsIgnorableErrorHandler<TUFromErrorHandler>, TUFromEncoding>;
      using TToProgressHandler =
        Impl::ProgressHandler<IsIgnorableErrorHandler<TUToErrorHandler>, TUToEncoding>;
      using TIntermediateOutputValueType = code_unit_t<TUToEncoding>;
      constexpr std::size_t MinimumIntermediateOutputMax =
        MaxTranscodeCodeUnits<TUFromEncoding, TUToEncoding>;
      constexpr std::size_t IntermediateOutputMax =
        ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(TIntermediateOutputValueType)
            < MinimumIntermediateOutputMax
          ? MinimumIntermediateOutputMax
          : ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(TIntermediateOutputValueType);
      using TInitialInput = Krys::Ranges::csubrange_for_t<TInput>;
      using TIntermediateOutput = Krys::Ranges::subrange<TIntermediateOutputValueType *>;
      using TTranscodeResult = decltype(Krys::TranscodeIntoRaw(
        std::declval<TInitialInput>(), fromEncoding, std::declval<TIntermediateOutput>(), toEncoding,
        fromErrorHandler, toErrorHandler, fromState, toState, pivot));
      using TWorkingInput = decltype(std::declval<TTranscodeResult>().Input);

      static_assert(
        Impl::DecodeLosslessOrDeliberate<remove_cvref_t<TFromEncoding>, remove_cvref_t<TFromErrorHandler>>,
        ZTD_TEXT_LOSSY_TRANSCODE_DECODE_MESSAGE_I_);
      static_assert(
        Impl::EncodeLosslessOrDeliberate<remove_cvref_t<TToEncoding>, remove_cvref_t<TToErrorHandler>>,
        ZTD_TEXT_LOSSY_TRANSCODE_ENCODE_MESSAGE_I_);

      TWorkingInput workingInput(Krys::Ranges::cbegin(input), Krys::Ranges::cend(input));
      TIntermediateOutputValueType intermediateOutputStorage[IntermediateOutputMax] {};
      TFromProgressHandler fromProgressHandler {};
      TToProgressHandler toProgressHandler {};
      std::size_t errorCount = 0;
      std::size_t pivotErrorCount = 0;
      for (;;)
      {
        fromProgressHandler.clear();
        toProgressHandler.clear();
        TIntermediateOutput intermediateOutput(intermediateOutputStorage, IntermediateOutputMax);
        auto result =
          Krys::TranscodeIntoRaw(std::move(workingInput), fromEncoding, intermediateOutput, toEncoding,
                                 fromProgressHandler, toProgressHandler, fromState, toState, pivot);
        std::size_t intermediateWrittenCount =
          static_cast<std::size_t>(result.Output.data() - intermediateOutput.data());
        TIntermediateOutput intermediateWritten(intermediateOutput.begin(),
                                                intermediateOutput.begin() + intermediateWrittenCount);
        Krys::Ranges::Impl::ContainerInsertBulk(output, intermediateWritten);
        if (result.ErrorCode == EncodingError::InsufficientOutputSpace)
        {
          if (toProgressHandler.CodeUnitsProgressSize() != 0)
          {
            Krys::Ranges::Impl::ContainerInsertBulk(output, toProgressHandler.CodeUnitsProgress());
            errorCount += result.ErrorCount;
            pivotErrorCount += result.PivotErrorCount;
            workingInput = Impl::UpdateConstInput<TWorkingInput>(std::move(result.Input));
            continue;
          }
          else if (result.PivotErrorCode == EncodingError::OK)
          {
            // If this occured, we need to record the original pivot position, and then try to
            // re-serialize with enough space all over again to avoid issues.
            std::size_t pivotRemnantCount =
              static_cast<std::size_t>(Krys::Ranges::size(pivot) - Krys::Ranges::size(result.Pivot));
            auto pivotRemnant =
              Krys::Ranges::reconstruct(std::in_place_type<TPivot>, Krys::Ranges::cbegin(pivot),
                                        Krys::Ranges::cbegin(pivot) + pivotRemnantCount);
            auto pivotResult =
              ::EncodeIntoRaw(pivotRemnant, toEncoding, intermediateOutput, toErrorHandler, toState);
            std::size_t intermediateWrittenCount =
              static_cast<std::size_t>(pivotResult.Output.data() - intermediateOutput.data());
            TIntermediateOutput pivotIntermediateOutput(
              intermediateOutput.begin(), intermediateOutput.begin() + intermediateWrittenCount);
            Krys::Ranges::Impl::ContainerInsertBulk(output, pivotIntermediateOutput);
            errorCount += pivotResult.ErrorCount;
            pivotErrorCount += result.PivotErrorCount;
            if (pivotResult.ErrorCode == EncodingError::OK)
            {
              workingInput = Impl::UpdateConstInput<TWorkingInput>(std::move(result.Input));
              continue;
            }
          }
          else
          {
            // it's okay, just loop around, we've got S P A C E for more
            workingInput = Impl::UpdateConstInput<TWorkingInput>(std::move(result.Input));
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
            auto errorResult = Krys::PropagateTranscodeDecodeError<TTranscodeResult>(
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
            Krys::Ranges::Impl::ContainerInsertBulk(output, errorIntermediateOutput);
            errorCount += errorResult.ErrorCount;
            pivotErrorCount += errorResult.PivotErrorCount;
            if (errorResult.ErrorCode != EncodingError::OK)
            {
              return TTranscodeResult(std::move(errorResult.Input), std::move(errorResult.Output),
                                      errorResult.FromState, errorResult.ToState, errorResult.ErrorCode,
                                      errorCount, std::move(errorResult.Pivot), errorResult.PivotErrorCode,
                                      pivotErrorCount);
            }
            workingInput = Impl::UpdateConstInput<TWorkingInput>(std::move(errorResult.Input));
          }
          else
          {
            // just need to call the second edge of the failure
            auto errorResult = Krys::PropagateTranscodeEncodeError<TTranscodeResult>(
              toEncoding,
              TErrorDecodeResult(std::move(result.Input), std::move(result.Pivot), fromState,
                                 result.PivotErrorCode, result.PivotErrorCount),
              TErrorEncodeResult(std::move(result.Pivot), intermediateOutput, toState, result.ErrorCode,
                                 result.ErrorCount),
              toErrorHandler, toProgressHandler.CodePointsProgress(), toProgressHandler.CodeUnitsProgress());
            std::size_t errorWrittenCount = errorResult.Output.data() - intermediateOutput.data();
            TIntermediateOutput errorIntermediateOutput(intermediateOutput.begin(),
                                                        intermediateOutput.begin() + errorWrittenCount);
            Krys::Ranges::Impl::ContainerInsertBulk(output, errorIntermediateOutput);
            errorCount += errorResult.ErrorCount;
            pivotErrorCount += errorResult.PivotErrorCount;
            if (errorResult.ErrorCode != EncodingError::OK)
            {
              return TTranscodeResult(std::move(errorResult.Input), std::move(errorResult.Output),
                                      errorResult.FromState, errorResult.ToState, errorResult.ErrorCode,
                                      errorCount, std::move(errorResult.Pivot), errorResult.PivotErrorCode,
                                      pivotErrorCount);
            }
            workingInput = Impl::UpdateConstInput<TWorkingInput>(std::move(errorResult.Input));
          }
          continue;
        }
        if (Krys::Ranges::empty(result.Input) && Krys::IsStateComplete(fromEncoding, fromState)
            && Krys::IsStateComplete(toEncoding, toState))
        {
          errorCount += result.ErrorCount;
          pivotErrorCount += result.PivotErrorCount;
          return TTranscodeResult(std::move(result.Input), std::move(result.Output), result.FromState,
                                  result.ToState, result.ErrorCode, errorCount, std::move(result.Pivot),
                                  result.PivotErrorCode, pivotErrorCount);
        }
        workingInput = Impl::UpdateConstInput<TWorkingInput>(std::move(result.Input));
      }
    }

    template <bool OutputOnly, bool NoState, typename TOutputContainer, typename TInput,
              typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
              typename TToErrorHandler, typename TFromState, typename TToState, typename TPivot>
    constexpr auto TranscodeDispatch(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                     TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                     TFromState &fromState, TToState &toState, TPivot &&pivot)
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
      auto statefulResult = Impl::IntermediateTranscodeToStorage(
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
        return Impl::ReplaceTranscodeResultOutputNoState(std::move(statefulResult), std::move(output));
      }
      else
      {
        return Impl::ReplaceTranscodeResultOutput(std::move(statefulResult), std::move(output));
      }
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A PivotlessTranscodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to transcode by first decoding the input code
  /// units to code points, then encoding the intermediate code points to the desired, output code units.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState,
                               TPivot &&pivot)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::TranscodeIntoRaw(
      std::move(reconstructedInput), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, decltype(result.Input) &&>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, decltype(result.Output) &&>;
    using TResult = TranscodeResult<TReconstructedResultInput, TReconstructedResultOutput, TFromState,
                                    TToState, decltype(result.Pivot)>;
    return TResult(Impl::SpanReconstruct<TInput>(std::move(result.Input)),
                   Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.FromState,
                   result.ToState, result.ErrorCode, result.ErrorCount, std::move(result.Pivot),
                   result.PivotErrorCode, result.PivotErrorCount);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @result A PivotlessTranscodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextTranscode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to transcode by first decoding the input code
  /// units to code points, then encoding the intermediate code points to the desired, output code units.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivotRange = Krys::Ranges::subrange<TCodePoint *>;

    constexpr std::size_t pivotBufferBufferMax =
      ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<TUFromEncoding>)
          < MaxCodePoints<TUFromEncoding>
        ? MaxCodePoints<TUFromEncoding>
        : ZTD_TEXT_INTERMEDIATE_TRANSCODE_BUFFER_SIZE_I_(code_point_t<TUFromEncoding>);

    TCodePoint pivotBuffer[pivotBufferBufferMax] {};
    TPivotRange pivot(pivotBuffer);
    return Krys::TranscodeInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
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
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @result A StatelessTranscodeResult object.
  /// @remarks This function calls the base reference, the TranscodeInto after creating a `ToState`
  /// from Krys::CreateEncodeState. The result from this function returns a
  /// StatelessTranscodeResult as opposed to a PivotlessTranscodeResult because the state
  /// information is on the stack, and returning the state in those types by reference will result in
  /// references to memory that has already been cleaned up. If you need access to the state parameters, call
  /// the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = encode_state_t<TUToEncoding>;

    TToState toState = ::Krys::CreateEncodeState(toEncoding);

    TToState toState = ::Krys::CreateEncodeState(toEncoding);
    auto statefulResult = ::TranscodeInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts the code units of the given input view through the
  ///        from encoding to code units of the to encoding into the output
  ///        view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @remarks This function creates a decode state `FromState` by calling CreateDecodeState. The
  /// result from this function returns a StatelessTranscodeResult as opposed to a
  /// PivotlessTranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = decode_state_t<TUFromEncoding>;

    TFromState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::TranscodeInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                               std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                               std::forward<TFromErrorHandler>(fromErrorHandler),
                               std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the
  ///        from encoding to code units of the to encoding into the output
  ///        view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// PivotlessTranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::TranscodeInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                               std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                               std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the
  ///        from encoding to code units of the to encoding into the output
  ///        view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// PivotlessTranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto TranscodeInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                               std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the
  ///        from encoding to code units of the to encoding into the output
  ///        view.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     output An output_view to write code units to as the result of the encode operation from
  /// the intermediate code points.
  /// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
  /// marked as careless to pass to the next function overload; and, a `FromEncoding` to interpret the
  /// `input` by checking the `input` 's `value_type.` This matters for lossy conversions that are not
  /// injective. The result from this function returns a StatelessTranscodeResult as opposed to a
  /// PivotlessTranscodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto TranscodeInto(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUFromEncoding = default_code_unit_encoding_t<Krys::Ranges::range_value_type_t<TUInput>>;

    TUFromEncoding fromEncoding {};
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeInto(std::forward<TInput>(input), fromEncoding, std::forward<TOutput>(output),
                               std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @returns A TranscodeResult object that contains references to `fromState` and @p
  /// toState and an `output` parameter that contains the `TOutputContainer` specified. If the container has a
  /// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the
  /// container, to aid with `push_back` / `insert` reallocation pains.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUToEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodeUnit>;
      return Impl::TranscodeDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodeUnit>, TOutputContainer>;
      return Impl::TranscodeDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @returns A PivotlessTranscodeResult object that contains references to `fromState` and @p
  /// toState and an `output` parameter that contains the `TOutputContainer` specified. If the container has a
  /// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the
  /// container, to aid with `push_back` / `insert` reallocation pains.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivotRange = Krys::Ranges::subrange<TCodePoint *>;

    constexpr std::size_t pivotBufferBufferMax = MaxCodePoints<TUFromEncoding>;

    TCodePoint pivotBuffer[pivotBufferBufferMax] {};
    TPivotRange pivot(pivotBuffer);
    return Krys::TranscodeTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A default state for the encode step of the operation is create using Krys::CreateEncodeState.
  /// The return type is stateless since both states must be passed in. If you want to have access to the
  /// states, create both of them yourself and pass them into a lower-level function that accepts those
  /// parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = encode_state_t<TUToEncoding>;

    TToState toState = ::Krys::CreateEncodeState(toEncoding);

    return Krys::TranscodeTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A default state for the decode step of the operation is create using CreateDecodeState. The
  /// return type is stateless since both states must be passed in. If you want to have access to the states,
  /// create both of them yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = decode_state_t<TUFromEncoding>;

    TFromState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::TranscodeTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `to_error_handler` for the encode step of the operation is created using default construction
  /// of a Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states
  /// must be passed in. If you want to have access to the states, create both of them yourself and pass them
  /// into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::TranscodeTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `from_error_handler` for the encode step of the operation is created using default
  /// construction of a Handlers::DefaultHandler that is marked as careless. The return type is stateless
  /// since both states must be passed in. If you want to have access to the states, create both of them
  /// yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto TranscodeTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::TranscodeTo<TOutputContainer>(std::forward<TInput>(input),
                                               std::forward<TFromEncoding>(fromEncoding),
                                               std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @returns A StatelessTranscodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `from_error_handler` for the encode step of the operation is created using default
  /// construction of a Handlers::DefaultHandler that is marked as careless. The return type is stateless
  /// since both states must be passed in. If you want to have access to the states, create both of them
  /// yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TToEncoding>
  constexpr auto TranscodeTo(TInput &&input, TToEncoding &&toEncoding)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      using TUFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      Handlers::DefaultHandler handler {};
      TUFromEncoding fromEncoding {};
      return Krys::TranscodeTo<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                 std::forward<TToEncoding>(toEncoding), handler);
    }
    else
    {
      using TUFromEncoding = default_code_unit_encoding_t<TCodeUnit>;
      Handlers::DefaultHandler handler {};
      TUFromEncoding fromEncoding {};
      return Krys::TranscodeTo<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                 std::forward<TToEncoding>(toEncoding), handler);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeTo or TranscodeIntoRaw.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodeUnit = code_unit_t<TUToEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodeUnit> || IsUnicodeCodePoint<TOutputCodeUnit>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      // prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
      using TRealOutputContainer = std::basic_string<TOutputCodeUnit>;
      return Impl::TranscodeDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, std::vector<TOutputCodeUnit>, TOutputContainer>;
      return Impl::TranscodeDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's encode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeTo or TranscodeIntoRaw.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivotRange = Krys::Ranges::subrange<TCodePoint *>;

    constexpr std::size_t pivotBufferBufferMax = MaxCodePoints<TUFromEncoding>;

    TCodePoint pivotBuffer[pivotBufferBufferMax] {};
    TPivotRange pivot(pivotBuffer);
    return Krys::Transcode<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's decode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeTo or TranscodeIntoRaw.
  /// @remarks This function creates an `ToState` for the encoding step of the operation using
  /// Krys::CreateEncodeState.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = encode_state_t<TUToEncoding>;

    TToState toState = ::Krys::CreateEncodeState(toEncoding);

    return Krys::Transcode<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's encode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeTo or TranscodeIntoRaw.
  /// @remarks This function creates an `FromState` for the encoding step of the operation using
  /// CreateDecodeState.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = decode_state_t<TUFromEncoding>;

    TFromState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::Transcode<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's decode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeTo or TranscodeIntoRaw.
  /// @remarks This function creates a `to_error_handler` from a class like Handlers::DefaultHandler, but that
  /// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that
  /// are not injective.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::Transcode<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to decode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeTo or TranscodeIntoRaw.
  /// @remarks This function creates a `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto Transcode(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::Transcode<TOutputContainer>(std::forward<TInput>(input),
                                             std::forward<TFromEncoding>(fromEncoding),
                                             std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a @c
  /// std::basic_string or a `std::vector` of some sort.
  /// @param[in]     input An input_view to read code units from and use in the decode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to encode the intermediate code points into the
  /// final code units.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// TranscodeTo or TranscodeIntoRaw.
  /// @remarks This function creates both: a `from_error_handler` from a class like Handlers::DefaultHandler,
  /// but that is marked as careless since you did not explicitly provide it; and, a `FromEncoding` derived
  /// from the `input`'s `value_type.` The careless marking matters for lossy conversions that are not
  /// injective.
  template <typename TOutputContainer = void, typename TInput, typename TToEncoding>
  constexpr auto Transcode(TInput &&input, TToEncoding &&toEncoding)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      using TUFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TUFromEncoding fromEncoding {};
      return Krys::Transcode<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                               std::forward<TToEncoding>(toEncoding));
    }
    else
    {
      using TUFromEncoding = default_code_unit_encoding_t<TCodeUnit>;

      TUFromEncoding fromEncoding {};
      return Krys::Transcode<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                               std::forward<TToEncoding>(toEncoding));
    }
  }
}
