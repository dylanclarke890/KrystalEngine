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
#include "Krystal.Text/EncodeOne.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/ForwardIfMoveOnly.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/Impl/ProgressHandler.hpp"
#include "Krystal.Text/Impl/SpanReconstruct.hpp"
#include "Krystal.Text/InlineContainers/InlineString.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/IsRedundant.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/MaxUnits.hpp"
#include "Krystal.Text/RecodeResult.hpp"
#include "Krystal.Text/State.hpp"

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
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A RecodeResult object that contains references to `fromState` and @p toState.
  /// @remark This function detects whether or not the ADL extension point `TextRecode` can be called with
  /// the provided parameters. If so, it will use that ADL extension point over the default implementation.
  /// Otherwise, it will loop over the two encodings and attempt to recode by first decoding the input code
  /// units to code points, then encoding the intermediate code points to the desired, output code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto BasicRecodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
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
                 .EncodeOne(std::forward<TInput>(input), std::forward<TPivot>(pivot),
                            std::declval<TFromProgressHandlerRef>(), fromState));
    using TResultPivot = decltype(std::declval<TIntermediateResult>().Output);
    using TEndResult = decltype(std::forward<TToEncoding>(toEncoding)
                                  .DecodeOne(std::declval<TResultPivot>(), std::declval<TWorkingOutput>(),
                                             std::declval<TToProgressHandlerRef>(), toState));
    using TWorkingIntermediate = decltype(std::declval<TEndResult>().Input);
    using TResultInput = decltype(std::declval<TIntermediateResult>().Input);
    using TResultOutput = decltype(std::declval<TEndResult>().Output);
    using TResult = RecodeResult<TResultInput, TResultOutput, TFromState, TToState, TResultPivot>;

    static_assert(
      Impl::EncodeLosslessOrDeliberate<remove_cvref_t<TFromEncoding>, remove_cvref_t<TFromErrorHandler>>,
      ZTD_TEXT_LOSSY_RECODE_ENCODE_MESSAGE_I_);
    static_assert(
      Impl::DecodeLosslessOrDeliberate<remove_cvref_t<TToEncoding>, remove_cvref_t<TToErrorHandler>>,
      ZTD_TEXT_LOSSY_RECODE_DECODE_MESSAGE_I_);

    auto savedInput = Krys::Ranges::SaveRange(input);
    TWorkingOutput workingOutput(std::forward<TOutput>(output));
    TFromProgressHandler fromIntermediateHandler(fromErrorHandler);
    TToProgressHandler toIntermediateHandler(toErrorHandler);
    auto intermediateResult = std::forward<TFromEncoding>(fromEncoding)
                                .EncodeOne(std::forward<TInput>(input), std::forward<TPivot>(pivot),
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
        return Krys::PropagateRecodeEncodeErrorWith<TResult>(
          Krys::Ranges::RestoreRange(std::move(savedInput), std::move(intermediateResult.Input)),
          std::move(workingOutput), fromEncoding, toEncoding, std::move(intermediateResult), fromErrorHandler,
          toErrorHandler, toState, fromIntermediateHandler.CodePointsProgress(),
          fromIntermediateHandler.CodeUnitsProgress(), toIntermediateHandler.CodeUnitsProgress(),
          toIntermediateHandler.CodePointsProgress());
      }
    }
    using TSpan = Span<const Krys::Ranges::range_value_type_t<TWorkingIntermediate>>;
    auto workingIntermediate = Krys::Ranges::cascading_reconstruct<TSpan, TWorkingIntermediate>(
      Krys::Ranges::cbegin(pivot), Krys::Ranges::cbegin(intermediateResult.Output));
    std::size_t errorCount = intermediateResult.ErrorCount;
    for (;;)
    {
      auto endResult = std::forward<TToEncoding>(toEncoding)
                         .DecodeOne(std::move(workingIntermediate), std::move(workingOutput),
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
          return Krys::PropagateRecodeDecodeErrorWith<TResult>(
            Krys::Ranges::RestoreRange(std::move(savedInput), std::move(intermediateResult.Input)),
            toEncoding, std::move(intermediateResult), std::move(endResult), toErrorHandler,
            toIntermediateHandler.CodeUnitsProgress(), toIntermediateHandler.CodePointsProgress());
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
      workingIntermediate = std::move(endResult.Input);
      workingOutput = std::move(endResult.Output);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A RecodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to recode by first decoding
  /// the input code units to code points, then encoding the intermediate code points to the desired, output
  /// code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto RecodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState,
                                  TPivot &&pivot)
  {
    if constexpr (Impl::HasTextRecodeOne<TInput, TOutput, TFromEncoding, TToEncoding, TFromErrorHandler,
                                         TToErrorHandler, TFromState, TToState, TPivot>)
    {
      return TextRecodeOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
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
      if constexpr (IsEncodeRedundant<TUFromEncoding, TUToEncoding>    // cf
                    && IsDecodeRedundant<TUFromEncoding, TUToEncoding> // cf
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
          Impl::reconstruct_pivot_recode_result_t<TUInput, TUOutput, TFromState, TToState, TUPivot>;
        return TResult(Krys::Ranges::reconstruct(std::in_place_type<TUInput>, std::move(result.in)),
                       Krys::Ranges::reconstruct(std::in_place_type<TUOutput>, std::move(result.out)),
                       fromState, toState, EncodingError::OK, 0, std::forward<TPivot>(pivot),
                       EncodingError::OK, 0);
      }
      else if constexpr (Impl::HasInternalTextRecodeOne<TInput, TFromEncoding, TOutput, TToEncoding,
                                                        TFromErrorHandler, TToErrorHandler, TFromState,
                                                        TToState, TPivot>)
      {
        return InternalTextRecodeOne(
          Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
          std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
          std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
          std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
          fromState, toState, std::forward<TPivot>(pivot));
      }
      else
      {
        return Krys::BasicRecodeOneIntoRaw(
          std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
          std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
          std::forward<TFromErrorHandler>(fromErrorHandler), std::forward<TToErrorHandler>(toErrorHandler),
          fromState, toState, std::forward<TPivot>(pivot));
      }
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @result A RecodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to recode by first decoding
  /// the input code units to code points, then encoding the intermediate code points to the desired, output
  /// code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto RecodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodeUnit = code_unit_t<TUFromEncoding>;
    using TPivot = Span<TCodeUnit>;
    constexpr std::size_t intermediateBufferMax = MaxCodeUnits<TUFromEncoding> * 2;

    TCodeUnit intermediate[intermediateBufferMax] {};
    TPivot pivot(intermediate);
    return Krys::RecodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler),
                                  std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @result A StatelessRecodeResult object.
  /// @remarks This function calls the base reference, the RecodeOneIntoRaw after creating a
  /// `ToState` from CreateDecodeState. The result from this function returns a
  /// StatelessRecodeResult as opposed to a RecodeResult because the state
  /// information is on the stack, and returning the state in those types by reference will result in
  /// references to memory that has already been cleaned up. If you need access to the state parameters, call
  /// the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto RecodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = decode_state_t<TUToEncoding>;

    TToState toState = Krys::CreateDecodeState(toEncoding);

    auto statefulResult = Krys::RecodeOneIntoRaw(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  ///
  /// @remarks This function creates a encode state `FromState` by calling Krys::CreateEncodeState. The
  /// result from this function returns a StatelessRecodeResult as opposed to a
  /// RecodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto RecodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                  TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = encode_state_t<TUFromEncoding>;

    TFromState fromState = ::Krys::CreateEncodeState(fromEncoding);

    return Krys::RecodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler),
                                  std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  ///
  /// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessRecodeResult as opposed to a
  /// RecodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto RecodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::RecodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  ///
  /// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessRecodeResult as opposed to a
  /// RecodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto RecodeOneIntoRaw(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                  TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::RecodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                  std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                                  handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  ///
  /// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
  /// marked as careless to pass to the next function overload; and, a `FromEncoding` to interpret the
  /// `input` by checking the `input` 's `value_type.` This matters for lossy conversions that are not
  /// injective. The result from this function returns a StatelessRecodeResult as opposed to a RecodeResult
  /// because the state information is on the stack, and returning the state in those types by reference will
  /// result in references to memory that has already been cleaned up. If you need access to the state
  /// parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto RecodeOneIntoRaw(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUFromEncoding = default_code_point_encoding_t<Krys::Ranges::range_value_type_t<TUInput>>;

    TUFromEncoding fromEncoding {};
    Handlers::DefaultHandler handler {};

    return Krys::RecodeOneIntoRaw(std::forward<TInput>(input), fromEncoding, std::forward<TOutput>(output),
                                  std::forward<TToEncoding>(toEncoding), handler);
  }

  namespace Impl
  {
    template <typename TOutputContainer, typename TInput, typename TFromEncoding, typename TToEncoding,
              typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
              typename TPivot>
    constexpr auto IntermediateRecodeOneToStorage(TInput &&input, TFromEncoding &&fromEncoding,
                                                  TOutputContainer &output, TToEncoding &&toEncoding,
                                                  TFromErrorHandler &&fromErrorHandler,
                                                  TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                                  TToState &toState, TPivot &&pivot)
    {
      using TUFromEncoding = remove_cvref_t<TFromEncoding>;
      using TUToEncoding = remove_cvref_t<TToEncoding>;
      using TUFromErrorHandler = remove_cvref_t<TFromErrorHandler>;
      using TUToErrorHandler = remove_cvref_t<TToErrorHandler>;
      constexpr std::size_t intermediateBufferMax = MaxCodePoints<TUToEncoding> * 2;
      using TIntermediateValue = code_point_t<TUToEncoding>;
      using TOutput = Span<TIntermediateValue>;

      static_assert(Impl::EncodeLosslessOrDeliberate<TUFromEncoding, TUFromErrorHandler>,
                    ZTD_TEXT_LOSSY_RECODE_ENCODE_MESSAGE_I_);
      static_assert(Impl::DecodeLosslessOrDeliberate<TUToEncoding, TUToErrorHandler>,
                    ZTD_TEXT_LOSSY_RECODE_DECODE_MESSAGE_I_);

      TIntermediateValue intermediateTranslationBuffer[intermediateBufferMax] {};
      TOutput intermediateInitialOutput(intermediateTranslationBuffer);
      auto result =
        Krys::RecodeOneIntoRaw(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
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
    constexpr auto RecodeOneDispatch(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
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
      auto statefulResult = IntermediateRecodeOneToStorage(
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
          return Impl::ReplaceRecodeResultOutputNoState(std::move(statefulResult), std::move(output));
        }
        else
        {
          return Impl::ReplaceRecodeResultOutput(std::move(statefulResult), std::move(output));
        }
      }
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A RecodeResult object that contains references to `fromState` and @p toState.
  /// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to recode by first decoding
  /// the input code units to code points, then encoding the intermediate code points to the desired, output
  /// code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto BasicRecodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                                    TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                                    TToErrorHandler &&toErrorHandler, TFromState &fromState,
                                    TToState &toState, TPivot &&pivot)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::BasicRecodeOneIntoRaw(
      std::move(reconstructedInput), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, TOutput>;
    return PivotlessRecodeResult<TReconstructedResultInput, TReconstructedResultOutput, TFromState, TToState>(
      Impl::SpanReconstruct<TInput>(std::move(result.Input)),
      Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.FromState, result.ToState,
      result.ErrorCode, result.ErrorCount);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @result A RecodeResult object that contains references to `fromState` and @p toState.
  /// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to recode by first decoding
  /// the input code units to code points, then encoding the intermediate code points to the desired, output
  /// code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto RecodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState,
                               TPivot &&pivot)
  {
    auto reconstructedInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));
    auto result = Krys::RecodeOneIntoRaw(
      std::move(reconstructedInput), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    using TReconstructedResultInput = Impl::span_reconstruct_t<TInput, decltype(result.Input)>;
    using TReconstructedResultOutput = Impl::span_reconstruct_mutable_t<TOutput, decltype(result.Output)>;
    return PivotlessRecodeResult<TReconstructedResultInput, TReconstructedResultOutput, TFromState, TToState>(
      Impl::SpanReconstruct<TInput>(std::move(result.Input)),
      Impl::SpanReconstructMutable<TOutput>(std::move(result.Output)), result.FromState, result.ToState,
      result.ErrorCode, result.ErrorCount);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view. Only performs one distinct unit of encoding.
  ///
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into
  /// the final code units.
  /// @param[in]     fromErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in]     toErrorHandler The error handlers for the from and to encodings,
  /// respectively.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @result A RecodeResult object that contains references to `fromState` and @p
  /// toState.
  /// @remark This function detects whether or not the ADL extension point `TextRecodeOne` can be called
  /// with the provided parameters. If so, it will use that ADL extension point over the default
  /// implementation. Otherwise, it will loop over the two encodings and attempt to recode by first decoding
  /// the input code units to code points, then encoding the intermediate code points to the desired, output
  /// code units.
  template <typename TInput, typename TOutput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto RecodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodeUnit = code_unit_t<TUFromEncoding>;
    using TPivot = Span<TCodeUnit>;
    constexpr std::size_t intermediateBufferMax = MaxCodeUnits<TUFromEncoding> * 2;

    TCodeUnit intermediate[intermediateBufferMax] {};
    TPivot pivot(intermediate);
    return Krys::RecodeOneInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                               std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                               std::forward<TFromErrorHandler>(fromErrorHandler),
                               std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @result A StatelessRecodeResult object.
  /// @remarks This function calls the base reference, the RecodeOneInto after creating a
  /// `ToState` from CreateDecodeState. The result from this function returns a
  /// StatelessRecodeResult as opposed to a RecodeResult because the state
  /// information is on the stack, and returning the state in those types by reference will result in
  /// references to memory that has already been cleaned up. If you need access to the state parameters, call
  /// the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto RecodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler, TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = decode_state_t<TUToEncoding>;

    TToState toState = Krys::CreateDecodeState(toEncoding);

    auto statefulResult = Krys::RecodeOneInto(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding), std::forward<TOutput>(output),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);

    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  ///
  /// @remarks This function creates a encode state `FromState` by calling Krys::CreateEncodeState. The
  /// result from this function returns a StatelessRecodeResult as opposed to a
  /// RecodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto RecodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler,
                               TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = encode_state_t<TUFromEncoding>;

    TFromState fromState = ::Krys::CreateEncodeState(fromEncoding);

    return Krys::RecodeOneInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                               std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                               std::forward<TFromErrorHandler>(fromErrorHandler),
                               std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  ///
  /// @remarks This function creates an `to_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessRecodeResult as opposed to a
  /// RecodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto RecodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding, TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::RecodeOneInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                               std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding),
                               std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @remarks This function creates an `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective. The result from this function returns a StatelessRecodeResult as opposed to a
  /// RecodeResult because the state information is on the stack, and returning the state in
  /// those types by reference will result in references to memory that has already been cleaned up. If you
  /// need access to the state parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding>
  constexpr auto RecodeOneInto(TInput &&input, TFromEncoding &&fromEncoding, TOutput &&output,
                               TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::RecodeOneInto(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                               std::forward<TOutput>(output), std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding into the output view.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     output An output_view to write code units to as the result of the decode operation from
  /// the intermediate code points.
  /// @remarks This function creates both: a `from_error_handler` using a Handlers::DefaultHandler that is
  /// marked as careless to pass to the next function overload; and, a `FromEncoding` to interpret the
  /// `input` by checking the `input` 's `value_type.` This matters for lossy conversions that are not
  /// injective. The result from this function returns a StatelessRecodeResult as opposed to a RecodeResult
  /// because the state information is on the stack, and returning the state in those types by reference will
  /// result in references to memory that has already been cleaned up. If you need access to the state
  /// parameters, call the lower-level functionality with your own created states.
  template <typename TInput, typename TToEncoding, typename TOutput>
  constexpr auto RecodeOneInto(TInput &&input, TToEncoding &&toEncoding, TOutput &&output)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TUFromEncoding = default_code_point_encoding_t<Krys::Ranges::range_value_type_t<TUInput>>;

    TUFromEncoding fromEncoding {};
    Handlers::DefaultHandler handler {};

    return Krys::RecodeOneInto(std::forward<TInput>(input), fromEncoding, std::forward<TOutput>(output),
                               std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @returns A RecodeResult object that contains references to `fromState` and @p
  /// toState and an `output` parameter that contains the `TOutputContainer` specified. If the container has a
  /// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the
  /// container, to aid with `push_back` / `insert` reallocation pains.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto RecodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodePoint = code_point_t<TUToEncoding>;
    constexpr std::size_t MinimumIntermediateOutputMax = MaxRecodeCodePoints<TUFromEncoding, TUToEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodePoint> || IsUnicodeCodePoint<TOutputCodePoint>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      using TRealOutputContainer = InlineBasicString<TOutputCodePoint, MinimumIntermediateOutputMax>;
      return Impl::RecodeOneDispatch<false, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodePoint, MinimumIntermediateOutputMax>,
                      TOutputContainer>;
      return Impl::RecodeOneDispatch<false, false, TRealOutputContainer>(
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
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @returns A RecodeResult object that contains references to `fromState` and @p
  /// toState and an `output` parameter that contains the `TOutputContainer` specified. If the container has a
  /// `container.reserve` function, it is and some multiple of the input's size is used to pre-size the
  /// container, to aid with `push_back` / `insert` reallocation pains.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto RecodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodeUnit = code_unit_t<TUFromEncoding>;
    using TPivot = Span<TCodeUnit>;
    constexpr std::size_t intermediateBufferMax = MaxCodeUnits<TUFromEncoding> * 2;

    TCodeUnit intermediate[intermediateBufferMax] {};
    TPivot pivot(intermediate);
    return Krys::RecodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @returns A StatelessRecodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A default state for the decode step of the operation is create using CreateDecodeState. The
  /// return type is stateless since both states must be passed in. If you want to have access to the states,
  /// create both of them yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto RecodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                             TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = decode_state_t<TUToEncoding>;

    TToState toState = Krys::CreateDecodeState(toEncoding);

    return Krys::RecodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @returns A StatelessRecodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A default state for the encode step of the operation is create using Krys::CreateEncodeState.
  /// The return type is stateless since both states must be passed in. If you want to have access to the
  /// states, create both of them yourself and pass them into a lower-level function that accepts those
  /// parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto RecodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = encode_state_t<TUFromEncoding>;

    TFromState fromState = ::Krys::CreateEncodeState(fromEncoding);

    return Krys::RecodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @returns A StatelessRecodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `to_error_handler` for the decode step of the operation is created using default construction
  /// of a Handlers::DefaultHandler that is marked as careless. The return type is stateless since both states
  /// must be passed in. If you want to have access to the states, create both of them yourself and pass them
  /// into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto RecodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                             TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::RecodeOneTo<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @returns A StatelessRecodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `from_error_handler` for the decode step of the operation is created using default
  /// construction of a Handlers::DefaultHandler that is marked as careless. The return type is stateless
  /// since both states must be passed in. If you want to have access to the states, create both of them
  /// yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto RecodeOneTo(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::RecodeOneTo<TOutputContainer>(std::forward<TInput>(input),
                                               std::forward<TFromEncoding>(fromEncoding),
                                               std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @returns A StatelessRecodeResult object that contains references to an `container.Output`
  /// parameter that contains the `TOutputContainer` specified.
  /// @remarks A `from_error_handler` for the decode step of the operation is created using default
  /// construction of a Handlers::DefaultHandler that is marked as careless. The return type is stateless
  /// since both states must be passed in. If you want to have access to the states, create both of them
  /// yourself and pass them into a lower-level function that accepts those parameters.
  template <typename TOutputContainer, typename TInput, typename TToEncoding>
  constexpr auto RecodeOneTo(TInput &&input, TToEncoding &&toEncoding)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      using TUFromEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      Handlers::DefaultHandler handler {};
      TUFromEncoding fromEncoding {};
      return Krys::RecodeOneTo<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                 std::forward<TToEncoding>(toEncoding), handler);
    }
    else
    {
      using TUFromEncoding = default_code_point_encoding_t<TCodePoint>;
      Handlers::DefaultHandler handler {};
      TUFromEncoding fromEncoding {};
      return Krys::RecodeOneTo<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                                 std::forward<TToEncoding>(toEncoding), handler);
    }
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) range as the intermediate
  /// pivot, usually a range of contiguous data from a span provided by the implementation but can be passed
  /// in here by the user.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// RecodeOneTo or RecodeOneInto.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState,
            typename TPivot>
  constexpr auto RecodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState, TToState &toState, TPivot &&pivot)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TUOutputContainer = remove_cvref_t<TOutputContainer>;
    using TOutputCodePoint = code_point_t<TUToEncoding>;
    constexpr std::size_t MinimumIntermediateOutputMax = MaxRecodeCodePoints<TUFromEncoding, TUToEncoding>;
    constexpr bool IsVoidContainer = Void<TUOutputContainer>;
    constexpr bool IsStringable = (IsCharTraitable<TOutputCodePoint> || IsUnicodeCodePoint<TOutputCodePoint>);
    if constexpr (IsVoidContainer && IsStringable)
    {
      using TRealOutputContainer = InlineBasicString<TOutputCodePoint, MinimumIntermediateOutputMax>;
      return Impl::RecodeOneDispatch<true, false, TRealOutputContainer>(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
        std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
    }
    else
    {
      using TRealOutputContainer =
        conditional_t<IsVoidContainer, InlineVector<TOutputCodePoint, MinimumIntermediateOutputMax>,
                      TOutputContainer>;
      return Impl::RecodeOneDispatch<true, false, TRealOutputContainer>(
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
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @param[in,out] toState A reference to the associated state for the `toEncoding` 's decode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// RecodeOneTo or RecodeOneInto.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState, typename TToState>
  constexpr auto RecodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState, TToState &toState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodeUnit = code_unit_t<TUFromEncoding>;
    using TPivot = Span<TCodeUnit>;
    constexpr std::size_t intermediateBufferMax = MaxCodeUnits<TUFromEncoding> * 2;

    TCodeUnit intermediate[intermediateBufferMax] {};
    TPivot pivot(intermediate);
    return Krys::RecodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState, pivot);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @param[in,out] fromState A reference to the associated state for the `fromEncoding` 's encode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// RecodeOneTo or RecodeOneInto.
  /// @remarks This function creates an `ToState` for the encoding step of the operation using
  /// CreateDecodeState.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler, typename TFromState>
  constexpr auto RecodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler,
                           TFromState &fromState)
  {
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TToState = decode_state_t<TUToEncoding>;

    TToState toState = Krys::CreateDecodeState(toEncoding);

    return Krys::RecodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState, toState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @param[in]     toErrorHandler The error handler for the `toEncoding` 's decode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// RecodeOneTo or RecodeOneInto.
  /// @remarks This function creates an `FromState` for the encoding step of the operation using
  /// Krys::CreateEncodeState.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler, typename TToErrorHandler>
  constexpr auto RecodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler, TToErrorHandler &&toErrorHandler)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TFromState = encode_state_t<TUFromEncoding>;

    TFromState fromState = ::Krys::CreateEncodeState(fromEncoding);

    return Krys::RecodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler),
      std::forward<TToErrorHandler>(toErrorHandler), fromState);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @param[in]     fromErrorHandler The error handler for the `fromEncoding` 's encode step.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// RecodeOneTo or RecodeOneInto.
  /// @remarks This function creates a `to_error_handler` from a class like Handlers::DefaultHandler, but that
  /// is marked as careless since you did not explicitly provide it. This matters for lossy conversions that
  /// are not injective.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding,
            typename TFromErrorHandler>
  constexpr auto RecodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding,
                           TFromErrorHandler &&fromErrorHandler)
  {
    auto handler = Handlers::Impl::DuplicateOrBeCareless(fromErrorHandler);

    return Krys::RecodeOne<TOutputContainer>(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), std::forward<TFromErrorHandler>(fromErrorHandler), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     fromEncoding The encoding that will be used to encode the input's code units into
  /// intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// RecodeOneTo or RecodeOneInto.
  /// @remarks This function creates a `from_error_handler` from a class like Handlers::DefaultHandler, but
  /// that is marked as careless since you did not explicitly provide it. This matters for lossy conversions
  /// that are not injective.
  template <typename TOutputContainer = void, typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto RecodeOne(TInput &&input, TFromEncoding &&fromEncoding, TToEncoding &&toEncoding)
  {
    Handlers::DefaultHandler handler {};

    return Krys::RecodeOne<TOutputContainer>(std::forward<TInput>(input),
                                             std::forward<TFromEncoding>(fromEncoding),
                                             std::forward<TToEncoding>(toEncoding), handler);
  }

  /// @brief Converts the code units of the given input view through the from encoding to code units of the to
  /// encoding for the output, which is then returned in a result structure with additional information about
  /// success.
  /// @tparam TOutputContainer The container to default-construct and serialize data into. Typically, a
  /// fixed-size container of some sort.
  /// @param[in]     input An input_view to read code units from and use in the encode operation that will
  /// produce intermediate code points.
  /// @param[in]     toEncoding The encoding that will be used to decode the intermediate code points into the
  /// final code units.
  /// @returns An `TOutputContainer` with the result, regardless of whether an error occurs or not. If you are
  /// looking for error information and not just a quick one-off conversion function, please use
  /// RecodeOneTo or RecodeOneInto.
  /// @remarks This function creates both: a `from_error_handler` from a class like Handlers::DefaultHandler,
  /// but that is marked as careless since you did not explicitly provide it; and, a `FromEncoding` derived
  /// from the @p "input"'s `value_type.` The careless marking matters for lossy conversions that are not
  /// injective.
  template <typename TOutputContainer = void, typename TInput, typename TToEncoding>
  constexpr auto RecodeOne(TInput &&input, TToEncoding &&toEncoding)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = Krys::Ranges::range_value_type_t<TUInput>;
    if (std::is_constant_evaluated())
    {
      using TUFromEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TUFromEncoding fromEncoding {};
      return Krys::RecodeOne<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                               std::forward<TToEncoding>(toEncoding));
    }
    else
    {
      using TUFromEncoding = default_code_point_encoding_t<TCodePoint>;

      TUFromEncoding fromEncoding {};
      return Krys::RecodeOne<TOutputContainer>(std::forward<TInput>(input), fromEncoding,
                                               std::forward<TToEncoding>(toEncoding));
    }
  }
}
