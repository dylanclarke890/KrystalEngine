#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/CountResult.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/ErrorHandlers/DefaultHandler.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/State.hpp"
#include <string_view>

namespace Krys
{
  /// @brief These functions use a variety of means to count the number of code units that will result from
  /// the input code units after a transcoding operation.

  /// @brief Counts the number of code units that will result from attempting an transcode operation on the
  /// input code points.
  /// @param[in] input The input range (of code units) to find out how many code units of the transcoded
  /// output there are.
  /// @param[in] fromEncoding The encoding that is going to be used to decode the input into an intermediary
  /// output.
  /// @param[in] toEncoding The encoding that is going to be used to encode the intermediary output.
  /// @param[in] fromErrorHandler The error handler to invoke when an intermediary decoding operation fails.
  /// @param[in] toErrorHandler The error handler to invoke when the final encoding operation fails.
  /// @param[in,out] fromState The state attached to the `fromEncoding` that will be used for the intermediary
  /// decode step.
  /// @param[in,out] toState The state related to the `toEncoding` that will be used for the final encoding
  /// step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) pivot range, usually a range
  /// of contiguous data from a span provided by the implementation but customizable by the end-user. If the
  /// intermediate conversion is what failed, then the pivot's `ErrorCode` member will be set to that
  /// error. This only happens if the overall operation also fails, and need not be checked unless to obtain
  /// additional information for when a top-level operation fails.
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `fromState` and `toState` .
  /// @remarks This method will not check any ADL extension points. A combination of implementation techniques
  /// will be used to count code units, with a loop over the `.EncodeOne` / `.DecodeOne` call into an
  /// intermediate, unseen buffer being the most basic choice.
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler, typename TFromState, typename TToState, typename TPivot>
  constexpr auto BasicCountAsTranscoded(TInput &&input, TFromEncoding &&fromEncoding,
                                        TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                        TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                        TToState &toState, TPivot &&pivot)
  {
    using TWorkingInput = Krys::Ranges::subrange_for_t<Impl::span_reconstruct_t<TInput, TInput>>;
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TResult = CountTranscodeResult<TWorkingInput, TFromState, TToState>;

    TWorkingInput workingInput(Impl::SpanReconstruct<TInput>(std::forward<TInput>(input)));

    std::size_t codeUnitCount = 0;
    std::size_t errorCount = 0;

#define KRYS_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY_CORE()                                                 \
  if (result.ErrorCode != EncodingError::OK)                                                                 \
  {                                                                                                          \
    return TResult(std::move(result.Input), codeUnitCount, result.FromState, result.ToState,                 \
                   result.ErrorCode, errorCount);                                                            \
  }                                                                                                          \
  codeUnitCount += result.Count;                                                                             \
  errorCount += result.ErrorCount;                                                                           \
  workingInput = std::move(result.Input);                                                                    \
  if (Krys::Ranges::empty(workingInput))                                                                     \
  {                                                                                                          \
    if (!Krys::IsStateComplete(fromEncoding, result.FromState))                                              \
    {                                                                                                        \
      continue;                                                                                              \
    }                                                                                                        \
    if (!Krys::IsStateComplete(toEncoding, result.ToState))                                                  \
    {                                                                                                        \
      continue;                                                                                              \
    }                                                                                                        \
    break;                                                                                                   \
  }                                                                                                          \
  do                                                                                                         \
  {                                                                                                          \
  } while (0)

#define KRYS_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY(...)                                                   \
  auto result =                                                                                              \
    __VA_ARGS__(Krys::Tag<TUFromEncoding, TUToEncoding> {}, std::move(workingInput), fromEncoding,           \
                toEncoding, fromErrorHandler, toErrorHandler, fromState, toState, pivot);                    \
  KRYS_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY_CORE()

    if constexpr (Impl::HasTextCountAsTranscodedOne<TWorkingInput, TFromEncoding, TToEncoding,
                                                    TFromErrorHandler, TToErrorHandler, TFromState, TToState,
                                                    TPivot>)
    {
      for (;;)
      {
        KRYS_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY(TextCountAsTranscodedOne);
      }
    }
    else if constexpr (Impl::HasInternalTextCountAsTranscodedOne<TWorkingInput, TFromEncoding, TToEncoding,
                                                                 TFromErrorHandler, TToErrorHandler,
                                                                 TFromState, TToState, TPivot>)
    {
      for (;;)
      {
        KRYS_TEXT_BASIC_COUNT_AS_TRANSCODED_LOOP_BODY(InternalTextCountAsTranscodedOne);
      }
    }
    else
    {
      constexpr std::size_t outputMax = MaxTranscodeCodeUnits<TUFromEncoding, TUToEncoding>;
      using TCodeUnit = code_unit_t<TUToEncoding>;

      TCodeUnit outputStorage[outputMax] {};
      Span<TCodeUnit, outputMax> output(outputStorage);

      for (;;)
      {
        auto result = Krys::TranscodeOneIntoRaw(std::move(workingInput), fromEncoding, output, toEncoding,
                                                fromErrorHandler, toErrorHandler, fromState, toState, pivot);
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), codeUnitCount, result.FromState, result.ToState,
                         result.ErrorCode, errorCount);
        }
        std::size_t written = static_cast<std::size_t>(result.Output.data() - output.data());
        codeUnitCount += written;
        errorCount += result.ErrorCount;
        workingInput = std::move(result.Input);
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
    }
    return TResult(std::move(workingInput), codeUnitCount, fromState, toState, EncodingError::OK, errorCount);
  }

  /// @brief Counts the number of code units that will result from attempting an transcode operation on the
  /// input code points.
  ///
  /// @param[in] input The input range (of code units) to find out how many code units of the transcoded
  /// output there are.
  /// @param[in] fromEncoding The encoding that is going to be used to decode the input into an intermediary
  /// output.
  /// @param[in] toEncoding The encoding that is going to be used to encode the intermediary output.
  /// @param[in] fromErrorHandler The error handler to invoke when an intermediary decoding operation fails.
  /// @param[in] toErrorHandler The error handler to invoke when the final encoding operation fails.
  /// @param[in,out] fromState The state attached to the `fromEncoding` that will be used for the intermediary
  /// decode step.
  /// @param[in,out] toState The state related to the `toEncoding` that will be used for the final encoding
  /// step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) pivot range, usually a range
  /// of contiguous data from a span provided by the implementation but customizable by the end-user. If the
  /// intermediate conversion is what failed, then the pivot's `ErrorCode` member will be set to that
  /// error. This only happens if the overall operation also fails, and need not be checked unless to obtain
  /// additional information for when a top-level operation fails.
  ///
  /// @returns A CountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler) and a
  /// reference to the provided `fromState` and `toState` .
  ///
  /// @remarks This method checks for the ADL extension point `TextCountAsTranscoded` . It will be called if
  /// it is possible. Otherwise, this function will defer to BasicCountAsTranscoded.
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler, typename TFromState, typename TToState, typename TPivot>
  constexpr auto CountAsTranscoded(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                   TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                   TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    if constexpr (Impl::HasTextCountAsTranscoded<TInput, TFromEncoding, TToEncoding, TFromErrorHandler,
                                                 TToErrorHandler, TFromState, TToState>)
    {
      return Krys::TextCountAsTranscoded(
        Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {}, std::forward<TInput>(input),
        std::forward<TFromEncoding>(fromEncoding), std::forward<TToEncoding>(toEncoding),
        std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
        fromState, toState, pivot);
    }
    else if constexpr (Impl::HasInternalTextCountAsTranscoded<TInput, TFromEncoding, TToEncoding,
                                                              TFromErrorHandler, TToErrorHandler, TFromState,
                                                              TToState>)
    {
      return Krys::InternalTextCountAsTranscoded(
        Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {}, std::forward<TInput>(input),
        std::forward<TFromEncoding>(fromEncoding), std::forward<TToEncoding>(toEncoding),
        std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
        fromState, toState, pivot);
    }
    else
    {
      return Krys::BasicCountAsTranscoded(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
  }

  /// @brief Counts the number of code units that will result from attempting an transcode operation on the
  /// input code points.
  /// @param[in] input The input range (of code units) to find out how many code units of the transcoded
  /// output there are.
  /// @param[in] fromEncoding The encoding that is going to be used to decode the input into an intermediary
  /// output.
  /// @param[in] toEncoding The encoding that is going to be used to encode the intermediary output.
  /// @param[in] fromErrorHandler The error handler to invoke when an intermediary decoding operation fails.
  /// @param[in] toErrorHandler The error handler to invoke when the final encoding operation fails.
  /// @param[in,out] fromState The state attached to the `fromEncoding` that will be used for the intermediary
  /// decode step.
  /// @param[in,out] toState The state attached to the `toEncoding` that will be used for the final encode
  /// step.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  /// @remarks This method will call CountAsTranscoded(input, FromEncoding, ToEncoding,
  /// from_error_handler, to_error_handler, FromState, ToState) with an `ToState` created by
  /// Krys::CreateEncodeState(ToEncoding).
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto CountAsTranscoded(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                   TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                   TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    using TPivot = Span<TCodePoint, MaxCodePoints<TUFromEncoding>>;
    TCodePoint intermediateStorage[MaxCodePoints<TUFromEncoding>] {};
    TPivot pivot(intermediateStorage);
    return Krys::CountAsTranscoded(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                   std::forward<TToEncoding>(toEncoding),
                                   std::forward<TFromErrorHandler>(fromErrorHandler),
                                   std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Counts the number of code units that will result from attempting an transcode operation on the
  /// input code points.
  /// @param[in] input The input range (of code units) to find out how many code units of the transcoded
  /// output there are.
  /// @param[in] fromEncoding The encoding that is going to be used to decode the input into an intermediary
  /// output.
  /// @param[in] toEncoding The encoding that is going to be used to encode the intermediary output.
  /// @param[in] fromErrorHandler The error handler to invoke when an intermediary decoding operation fails.
  /// @param[in] toErrorHandler The error handler to invoke when the final encoding operation fails.
  /// @param[in,out] fromState The state attached to the `fromEncoding` that will be used for the intermediary
  /// decode step.
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  /// @remarks This method will call CountAsTranscoded(input, FromEncoding, ToEncoding,
  /// from_error_handler, to_error_handler, FromState, ToState) with an `ToState` created by
  /// Krys::CreateEncodeState(ToEncoding).
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler, typename TFromState>
  constexpr auto CountAsTranscoded(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                   TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                                   TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TState = encode_state_t<TUToEncoding>;

    TState toState = Krys::CreateEncodeState(toEncoding);

    auto result = Krys::CountAsTranscoded(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
    return Impl::SliceToStateless(std::move(result));
  }

  /// @brief Counts the number of code units that will result from attempting an transcode operation on the
  /// input code points.
  ///
  /// @param[in] input The input range (of code units) to find out how many code units of the transcoded
  /// output there are.
  /// @param[in] fromEncoding The encoding that is going to be used to decode the input into an intermediary
  /// output.
  /// @param[in] toEncoding The encoding that is going to be used to encode the intermediary output.
  /// @param[in] fromErrorHandler The error handler to invoke when the decode portion of the transcode
  /// operation fails.
  /// @param[in] toErrorHandler The error handler to invoke when the encode portion of the transcode operation
  /// fails.
  ///
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  ///
  /// @remarks This method will call CountAsTranscoded(input, FromEncoding, ToEncoding,
  /// from_error_handler, to_error_handler, FromState) with an `FromState` created by
  /// CreateDecodeState(FromEncoding).
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler,
            typename TToErrorHandler>
  constexpr auto CountAsTranscoded(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                   TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TState = decode_state_t<TUFromEncoding>;

    TState fromState = Krys::CreateDecodeState(fromEncoding);

    return Krys::CountAsTranscoded(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                   std::forward<TToEncoding>(toEncoding),
                                   std::forward<TFromErrorHandler>(fromErrorHandler),
                                   std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Counts the number of code units that will result from attempting an transcode operation.
  ///
  /// @param[in] input The input range (of code units) to find out how many code units of the transcoded
  /// output there are.
  /// @param[in] fromEncoding The encoding that is going to be used to decode the input into an intermediary
  /// output.
  /// @param[in] toEncoding The encoding that is going to be used to encode the intermediary output.
  /// @param[in] fromErrorHandler The error handler to invoke when the decode portion of the transcode
  /// operation fails.
  ///
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  ///
  /// @remarks This method will call CountAsTranscoded(input, FromEncoding, ToEncoding,
  /// from_error_handler, to_error_handler) by creating an `to_error_handler` similar to
  /// Handlers::DefaultHandler.
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromErrorHandler>
  constexpr auto CountAsTranscoded(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                                   TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::CountAsTranscoded(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                   std::forward<TToEncoding>(toEncoding),
                                   std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Counts the number of code units that will result from attempting an transcode operation.
  ///
  /// @param[in] input The input range (of code units) to find out how many code units of the transcoded
  /// output there are.
  /// @param[in] fromEncoding The encoding that is going to be used to decode the input into an intermediary
  /// output.
  /// @param[in] toEncoding The encoding that is going to be used to encode the intermediary output.
  ///
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  ///
  /// @remarks This method will call CountAsTranscoded(Input, Encoding, ErrorHandler) by creating
  /// an `ErrorHandler` similar to Handlers::DefaultHandler.
  template <typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto CountAsTranscoded(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler fromErrorHandler {};
    return Krys::CountAsTranscoded(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                   std::forward<TToEncoding>(toEncoding), fromErrorHandler);
  }

  /// @brief Counts the number of code units that will result from attempting an transcode operation on the
  /// input code points.
  ///
  /// @param[in] input The input range (of code units) to find out how many code units of the transcoded
  /// output there are.
  /// @param[in] toEncoding The encoding that is going to be used to encode the input into an intermediary
  /// output.
  ///
  /// @returns A StatelessCountResult that includes information about how many code units are present,
  /// taking into account any invoked errors (like replacement from Handlers::ReplacementHandler).
  ///
  /// @remarks Calls CountAsTranscoded(Input, Encoding) with an `encoding` that is derived from
  /// DefaultCodeUnitEncoding.
  template <typename TInput, typename TToEncoding>
  constexpr auto CountAsTranscoded(TInput &&input, TToEncoding &&toEncoding)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = remove_cvref_t<Krys::Ranges::range_value_type_t<TUInput>>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TFromEncoding fromEncoding {};
      return Krys::CountAsTranscoded(std::forward<TInput>(input), fromEncoding,
                                     std::forward<TToEncoding>(toEncoding));
    }
    else
    {
      using TFromEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TFromEncoding fromEncoding {};
      return Krys::CountAsTranscoded(std::forward<TInput>(input), fromEncoding,
                                     std::forward<TToEncoding>(toEncoding));
    }
  }
}
