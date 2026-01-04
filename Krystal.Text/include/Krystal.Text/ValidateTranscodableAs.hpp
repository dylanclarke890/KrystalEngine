#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/ErrorHandlers/PassHandler.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/TranscodeOne.hpp"
#include "Krystal.Text/ValidateResult.hpp"
#include <algorithm>
#include <string_view>

namespace Krys
{
  /// @brief These functions check if the given input of code points will decode without an error. Note that
  /// this does not mean that an error handler will be invoked that can "smooth over" any possible errors:
  /// this checks solely if it will decode from code units into code points cleanly.

  /// @brief Validates the code units of the `input` according to the `fromEncoding` with the given states
  /// `decodeState` and `encodeState` to see if it can be turned into code points, and then code units again.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] fromEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in] toEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in, out] decodeState The state to use for the decoding portion of the validation check.
  /// @param[in, out] encodeState The state to use for the encoding portion of the validation check.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) pivot range, usually a range
  /// of contiguous data from a span provided by the implementation but customizable by the end-user. If the
  /// intermediate conversion is what failed, then the pivot's `ErrorCode` member will be set to that
  /// error. This only happens if the overall operation also fails, and need not be checked unless to obtain
  /// additional information for when a top-level operation fails.
  /// @remarks This function explicitly does not call any extension points. It defers to doing a typical loop
  /// over the code points to verify it can be decoded into code points, and then encoded back into code
  /// units, with no errors and with the exact same value sequence as the original.
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TDecodeState,
            typename TEncodeState, typename TPivot>
  constexpr auto BasicValidateTranscodableAs(TInput &&input, TFromEncoding &&fromEncoding,
                                             TToEncoding &&toEncoding, TDecodeState &decodeState,
                                             TEncodeState &encodeState, TPivot &&pivot)
  {
    using TInitialInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TWorkingInput = Krys::Ranges::subrange_for_t<TInitialInput>;
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TUToEncoding = remove_cvref_t<TToEncoding>;
    using TResult = ValidatePivotlessTranscodeResult<TWorkingInput, TDecodeState, TEncodeState>;

    TWorkingInput workingInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));

    if constexpr (Impl::HasTextValidateTranscodableAsOne<TWorkingInput, TFromEncoding, TToEncoding,
                                                         TDecodeState, TEncodeState, TPivot>)
    {
      (void)encodeState;
      for (;;)
      {
        auto result = TextValidateTranscodableAsOne(Krys::Tag<TUFromEncoding, TUToEncoding> {}, workingInput,
                                                    fromEncoding, toEncoding, decodeState, pivot);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, decodeState);
        }
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(fromEncoding, decodeState))
          {
            continue;
          }
          if (!Krys::IsStateComplete(toEncoding, encodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(Krys::Ranges::Reconstruct(std::in_place_type<TWorkingInput>, std::move(workingInput)),
                     true, decodeState, encodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateTranscodableAsOne<
                         TWorkingInput, TFromEncoding, TToEncoding, TDecodeState, TEncodeState, TPivot>)
    {
      (void)encodeState;
      for (;;)
      {
        auto result =
          InternalTextValidateTranscodableAsOne(Krys::Tag<TUFromEncoding, TUToEncoding> {}, workingInput,
                                                fromEncoding, toEncoding, decodeState, pivot);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, decodeState);
        }
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(fromEncoding, decodeState))
          {
            continue;
          }
          if (!Krys::IsStateComplete(toEncoding, encodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(Krys::Ranges::reconstruct(std::in_place_type<TWorkingInput>, std::move(workingInput)),
                     true, decodeState, encodeState);
    }
    else
    {
      using TCodeUnit = code_unit_t<TUToEncoding>;

      TCodeUnit outputStorage[MaxCodeUnits<TUToEncoding>] {};
      Span<TCodeUnit, MaxCodeUnits<TUToEncoding>> output(outputStorage);

      Handlers::PassHandler handler {};

      for (;;)
      {
        auto transcodeResult =
          Krys::TranscodeOneIntoRaw(std::move(workingInput), fromEncoding, output, toEncoding, handler,
                                    handler, decodeState, encodeState, pivot);
        if (transcodeResult.ErrorCode != EncodingError::OK)
        {
          return TResult(
            Krys::Ranges::reconstruct(std::in_place_type<TWorkingInput>, std::move(workingInput)), false,
            decodeState, encodeState);
        }
        workingInput = std::move(transcodeResult.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(fromEncoding, decodeState))
          {
            continue;
          }
          if (!::IsStateComplete(toEncoding, encodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(Krys::Ranges::reconstruct(std::in_place_type<TWorkingInput>, std::move(workingInput)),
                     true, decodeState, encodeState);
    }
  }

  /// @brief Validates the code units of the `input` according to the `encoding` with the given states @p
  /// decodeState and `encodeState` to see if it can be turned into code points.
  ///
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] fromEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in] toEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in, out] decodeState The state to use for the decoding portion of the validation check.
  /// @param[in, out] encodeState The state to use for the encoding portion of the validation check.
  /// @param[in, out] pivot A reference to a descriptor of a (potentially usable) pivot range, usually a range
  /// of contiguous data from a span provided by the implementation but customizable by the end-user. If the
  /// intermediate conversion is what failed, then the pivot's `ErrorCode` member will be set to that
  /// error. This only happens if the overall operation also fails, and need not be checked unless to obtain
  /// additional information for when a top-level operation fails.
  ///
  /// @remarks This functions checks to see if extension points for `TextValidateTranscodableAs` is available
  /// taking the available 4 parameters. If so, it calls this. Otherwise, it defers to
  /// ValidateTranscodableAs.
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TDecodeState,
            typename TEncodeState, typename TPivot>
  constexpr auto ValidateTranscodableAs(TInput &&input, TFromEncoding &&fromEncoding,
                                        TToEncoding &&toEncoding, TDecodeState &decodeState,
                                        TEncodeState &encodeState, TPivot &&pivot)
  {
    using TUFromEncoding = remove_cvref_t<TToEncoding>;
    using TUToEncoding = remove_cvref_t<TFromEncoding>;
    if constexpr (Impl::HasTextValidateTranscodableAs<TInput, TFromEncoding, TToEncoding, TDecodeState,
                                                      TEncodeState, TPivot>)
    {
      (void)encodeState;
      return TextValidateTranscodableAs(
        Krys::Tag<TUFromEncoding, TUToEncoding> {}, std::forward<TInput>(input),
        std::forward<TFromEncoding>(fromEncoding), std::forward<TToEncoding>(toEncoding), decodeState,
        encodeState, pivot);
    }
    else if constexpr (Impl::HasTextValidateTranscodableAs<TInput, TFromEncoding, TToEncoding, TDecodeState,
                                                           TEncodeState, TPivot>)
    {
      return TextValidateTranscodableAs(
        Krys::Tag<TUFromEncoding, TUToEncoding> {}, std::forward<TInput>(input),
        std::forward<TFromEncoding>(fromEncoding), std::forward<TToEncoding>(toEncoding), decodeState,
        encodeState, pivot);
    }
    else if constexpr (Impl::HasInternalTextValidateTranscodableAs<TInput, TFromEncoding, TToEncoding,
                                                                   TDecodeState, TEncodeState, TPivot>)
    {
      (void)encodeState;
      return InternalTextValidateTranscodableAs(
        Krys::Tag<TUFromEncoding, TUToEncoding> {}, std::forward<TInput>(input),
        std::forward<TFromEncoding>(fromEncoding), std::forward<TToEncoding>(toEncoding), decodeState,
        encodeState, pivot);
    }
    else
    {
      return Krys::BasicValidateTranscodableAs(
        std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
        std::forward<TToEncoding>(toEncoding), decodeState, encodeState, pivot);
    }
  }

  /// @brief Validates the code units of the `input` according to the `encoding` with the given states @p
  /// decodeState and `encodeState` to see if it can be turned into code points.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] fromEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in] toEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in, out] decodeState The state to use for the decoding portion of the validation check.
  /// @param[in, out] encodeState The state to use for the encoding portion of the validation check.
  /// @remarks This functions checks to see if extension points for `TextValidateTranscodableAs` is available
  /// taking the available 4 parameters. If so, it calls this. Otherwise, it defers to
  /// ValidateTranscodableAs.
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TDecodeState,
            typename TEncodeState>
  constexpr auto ValidateTranscodableAs(TInput &&input, TFromEncoding &&fromEncoding,
                                        TToEncoding &&toEncoding, TDecodeState &decodeState,
                                        TEncodeState &encodeState)
  {
    using TUFromEncoding = remove_cvref_t<TFromEncoding>;
    using TCodePoint = code_point_t<TUFromEncoding>;
    constexpr std::size_t maxPoints = MaxCodePoints<TUFromEncoding>;
    TCodePoint intermediate[maxPoints] {};
    Span<TCodePoint, maxPoints> pivot(intermediate);
    return Krys::ValidateTranscodableAs(
      std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
      std::forward<TToEncoding>(toEncoding), decodeState, encodeState, pivot);
  }

  /// @brief Validates the code units of the `input` according to the `fromEncoding` object with the given
  /// state `decodeState` to see if it can be turned into code units of the `toEncoding` object.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] fromEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in] toEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in, out] decodeState The state to use for the decoding portion of the validation check.
  /// @remarks Will create a default encode_state for `toEncoding`.
  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TDecodeState>
  constexpr auto ValidateTranscodableAs(TInput &&input, TFromEncoding &&fromEncoding,
                                        TToEncoding &&toEncoding, TDecodeState &decodeState)
  {
    auto encodeState = Krys::CreateEncodeState(toEncoding);
    auto result =
      Krys::ValidateTranscodableAs(std::forward<TInput>(input), std::forward<TFromEncoding>(fromEncoding),
                                   std::forward<TToEncoding>(toEncoding), decodeState, encodeState);
    return Impl::SliceToStateless(std::move(result));
  }

  /// @brief Validates the code units of the `input` according to the `fromEncoding` object to see if it can
  /// be turned into code units of the `toEncoding` object.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] fromEncoding The encoding to verify can properly encode the input of code units.
  /// @param[in] toEncoding The encoding to verify can properly encode the input of code units.
  /// @remarks Will create a default decode_state for `fromEncoding`.
  template <typename TInput, typename TFromEncoding, typename TToEncoding>
  constexpr auto ValidateTranscodableAs(TInput &&input, TFromEncoding &&fromEncoding,
                                        TToEncoding &&toEncoding)
  {
    auto decodeState = Krys::CreateDecodeState(fromEncoding);
    return Krys::ValidateTranscodableAs(std::forward<TInput>(input),
                                        std::forward<TFromEncoding>(fromEncoding),
                                        std::forward<TToEncoding>(toEncoding), decodeState);
  }

  /// @brief Validates the code units of the `input` according to the `fromEncoding` object to see if it can
  /// be turned into code units of the `toEncoding` object.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] toEncoding The encoding to verify can properly encode the input of code units.
  /// @remarks Will create a default encode_state for `toEncoding`.
  template <typename TInput, typename TToEncoding>
  constexpr auto ValidateTranscodableAs(TInput &&input, TToEncoding &&toEncoding)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = remove_cvref_t<Krys::Ranges::range_value_type_t<TUInput>>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TFromEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TFromEncoding fromEncoding {};
      return Krys::ValidateTranscodableAs(std::forward<TInput>(input), fromEncoding,
                                          std::forward<TToEncoding>(toEncoding));
    }
    else
    {
      using TFromEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TFromEncoding fromEncoding {};
      return Krys::ValidateTranscodableAs(std::forward<TInput>(input), fromEncoding,
                                          std::forward<TToEncoding>(toEncoding));
    }
  }
}
