#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Text/CharPredicates.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DefaultEncoding.hpp"
#include "Krystal.Text/ErrorHandlers/PassHandler.hpp"
#include "Krystal.Text/Impl/EncodingRange.hpp"
#include "Krystal.Text/Impl/IsLossless.hpp"
#include "Krystal.Text/MaxUnits.hpp"
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

  /// @brief Validates the code units of the `input` according to the `encoding` with the given states @p
  /// decodeState and `encodeState` to see if it can be turned into code points.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] encoding The encoding to verify can properly encode the input of code units.
  /// @param[in] decodeState The state to use for the decoding portion of the validation check.
  /// @param[in] encodeState The state to use for the encoding portion of the validation check.
  /// @remarks This function explicitly does not call any extension points. It defers to doing a typical loop
  /// over the code points to verify it can be decoded into code points, and then encoded back into code
  /// units, with no errors and with the exact same value sequence as the original.
  template <typename TInput, typename TEncoding, typename TDecodeState, typename TEncodeState>
  constexpr auto BasicValidateDecodableAs(TInput &&input, TEncoding &&encoding, TDecodeState &decodeState,
                                          TEncodeState &encodeState)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TInitialInput = Impl::span_reconstruct_t<TInput, TInput>;
    using TWorkingInput = Krys::Ranges::subrange_for_t<TInitialInput>;
    using TResult = ValidatePivotlessTranscodeResult<TWorkingInput, TDecodeState, TEncodeState>;

    TWorkingInput workingInput(Impl::SpanReconstruct<TInput>(std::forward<TInput>(input)));

    if constexpr (Impl::HasTextValidateDecodableAsOne<TWorkingInput, TEncoding, TDecodeState, TEncodeState>)
    {
      for (;;)
      {
        auto result = Krys::TextValidateDecodableAsOne(Krys::Tag<TUEncoding> {}, encoding,
                                                       std::move(workingInput), decodeState, encodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, decodeState, encodeState);
        }
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(encoding, decodeState))
          {
            continue;
          }
          if (!Krys::IsStateComplete(encoding, encodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(std::move(workingInput), true, decodeState, encodeState);
    }
    else if constexpr (Impl::HasTextValidateDecodableAsOne<TInitialInput, TEncoding, TDecodeState>)
    {
      for (;;)
      {
        auto result = TextValidateDecodableAsOne(Krys::Tag<TUEncoding> {}, encoding, std::move(workingInput),
                                                 decodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, decodeState, encodeState);
        }
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(encoding, decodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(std::move(workingInput), true, decodeState, encodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateDecodableAsOne<TInitialInput, TEncoding, TDecodeState,
                                                                   TEncodeState>)
    {
      for (;;)
      {
        auto result = InternalTextValidateDecodableAsOne(Krys::Tag<TUEncoding> {}, encoding,
                                                         std::move(workingInput), decodeState, encodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, decodeState, encodeState);
        }
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(encoding, decodeState))
          {
            continue;
          }
          if (!Krys::IsStateComplete(encoding, encodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(std::move(workingInput), true, decodeState, encodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateDecodableAsOne<TInitialInput, TEncoding, TDecodeState>)
    {
      for (;;)
      {
        auto result = InternalTextValidateDecodableAsOne(Krys::Tag<TUEncoding> {}, encoding,
                                                               std::move(workingInput), decodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, decodeState, encodeState);
        }
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(encoding, decodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(std::move(workingInput), true, decodeState, encodeState);
    }
    else
    {
      constexpr std::size_t codeUnitMax = MaxTranscodeCodeUnits<TUEncoding, TUEncoding>;
      constexpr std::size_t codePointMax = MaxCodePoints<TUEncoding>;
      using TCodeUnit = code_unit_t<TUEncoding>;
      using TCodePoint = code_point_t<TUEncoding>;
      using TPivot = Span<TCodePoint, codePointMax>;

      TCodePoint codePointBuf[codePointMax] {};
      TCodeUnit codeUnitBuf[codeUnitMax] {};
      Span<TCodeUnit, codeUnitMax> codeUnitView(codeUnitBuf);
      TPivot pivot(codePointBuf);

      for (;;)
      {
        auto result =
          Krys::TranscodeOneIntoRaw(std::move(workingInput), encoding, codeUnitView, encoding, Handlers::Pass,
                                    Handlers::Pass, decodeState, encodeState, pivot);
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), false, decodeState, encodeState);
        }
        const bool isTranscodeRoundtripOkay = Krys::Ranges::equal(
          Krys::Ranges::cbegin(workingInput), Krys::Ranges::cbegin(result.Input), codeUnitView.data(),
          std::to_address(Krys::Ranges::cbegin(result.Output)), ::equal_to_char);
        if (!isTranscodeRoundtripOkay)
        {
          return TResult(std::move(result.Input), false, decodeState, encodeState);
        }
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(encoding, decodeState))
          {
            continue;
          }
          if (!Krys::IsStateComplete(encoding, encodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(std::move(workingInput), true, decodeState, encodeState);
    }
  }

  /// @brief Validates the code units of the `input` according to the `encoding` with the given states @p
  /// decodeState and `encodeState` to see if it can be turned into code points.
  ///
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] encoding The encoding to verify can properly encode the input of code units.
  /// @param[in] decodeState The state to use for the decoding portion of the validation check.
  /// @param[in] encodeState The state to use for the encoding portion of the validation check.
  ///
  /// @remarks This functions checks to see if extension points for `TextValidateDecodableAs` is available
  /// taking the available 4 parameters. If so, it calls this. Otherwise, it defers to ValidateDecodableAs.
  template <typename TInput, typename TEncoding, typename TDecodeState, typename TEncodeState>
  constexpr auto ValidateDecodableAs(TInput &&input, TEncoding &&encoding, TDecodeState &decodeState,
                                     TEncodeState &encodeState)
  {
    if constexpr (Impl::HasTextValidateDecodableAs<TInput, TEncoding, TDecodeState>)
    {
      return TextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                           std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                           decodeState, encodeState);
    }
    else if constexpr (Impl::HasTextValidateDecodableAs<TInput, TEncoding, TDecodeState>)
    {
      (void)encodeState;
      return TextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                           std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                           decodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateDecodableAs<TInput, TEncoding, TDecodeState>)
    {
      return InternalTextValidateDecodableAs(
        Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
        std::forward<TEncoding>(encoding), decodeState, encodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateDecodableAs<TInput, TEncoding, TDecodeState>)
    {
      (void)encodeState;
      return InternalTextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                                   std::forward<TInput>(input),
                                                   std::forward<TEncoding>(encoding), decodeState);
    }
    else
    {
      return Krys::BasicValidateDecodableAs(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                            decodeState, encodeState);
    }
  }

  /// @brief Validates the code units of the `input` according to the `encoding` with the given state @p
  /// decodeState to see if it can be turned into code points.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] encoding The encoding to verify can properly encode the input of code units.
  /// @param[in] decodeState The state to use for the decoding portion of the validation check.
  /// @remarks This functions checks to see if extension points for `TextValidateDecodableAs` is available
  /// taking the available 3 parameters. If so, it calls this. Otherwise, it creates an encoding state through
  /// Krys::CreateEncodeState before calling ValidateDecodableAs(input, encoding,
  /// decodeState,
  /// encodeState).
  template <typename TInput, typename TEncoding, typename TDecodeState>
  constexpr auto ValidateDecodableAs(TInput &&input, TEncoding &&encoding, TDecodeState &decodeState)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    if constexpr (Impl::HasTextValidateDecodableAs<TInput, TEncoding, TDecodeState>)
    {
      return TextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                           std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                           decodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateDecodableAs<TInput, TEncoding, TDecodeState>)
    {
      return InternalTextValidateDecodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                                   std::forward<TInput>(input),
                                                   std::forward<TEncoding>(encoding), decodeState);
    }
    else
    {
      using TState = encode_state_t<TUEncoding>;

      TState encodeState = Krys::CreateEncodeStateWith(encoding, decodeState);
      auto statefulResult = Krys::ValidateDecodableAs(
        std::forward<TInput>(input), std::forward<TEncoding>(encoding), decodeState, encodeState);
      if constexpr (IsSpecializationOf<decltype(statefulResult), ValidateResult>)
      {
        return statefulResult;
      }
      else
      {
        return Impl::DropSingleState(std::move(statefulResult));
      }
    }
  }

  /// @brief Validates the code units of the `input` according to the `encoding` to see if they can be turned
  /// into code points.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @param[in] encoding The encoding to verify can properly encode the input of code units.
  /// @remarks This functions creates an encoding `state` through CreateDecodeState before calling
  /// the next overload of ValidateDecodableAs.
  template <typename TInput, typename TEncoding>
  constexpr auto ValidateDecodableAs(TInput &&input, TEncoding &&encoding)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = decode_state_t<TUEncoding>;

    TState state = Krys::CreateDecodeState(encoding);
    auto statefulResult =
      Krys::ValidateDecodableAs(std::forward<TInput>(input), std::forward<TEncoding>(encoding), state);
    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Validates the code units of the `input` to see if it can be turned into code points.
  /// @param[in] input The input range of code units to validate is possible for encoding into code points.
  /// @returns A StatelessValidateResult detailing whether or not the input code points can be turned
  /// into code units of the corresponding encoding.
  /// @remarks This functions creates an `encoding` by passing the `value_type` of the `input` range through
  /// DefaultCodeUnitEncoding.
  template <typename TInput>
  constexpr auto ValidateDecodableAs(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodeUnit = remove_cvref_t<Krys::Ranges::range_value_type_t<TUInput>>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::ValidateDecodableAs(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_unit_encoding_t<TCodeUnit>;
      TEncoding encoding {};
      return Krys::ValidateDecodableAs(std::forward<TInput>(input), encoding);
    }
  }
}
