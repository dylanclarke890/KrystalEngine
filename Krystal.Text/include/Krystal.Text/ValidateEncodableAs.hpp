#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
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
#include "Krystal.Text/RecodeOne.hpp"
#include "Krystal.Text/State.hpp"
#include "Krystal.Text/ValidateResult.hpp"
#include <algorithm>
#include <string_view>

namespace Krys
{
  /// @brief These functions check if the given input of code points will encode without an error. Note that
  /// this does not mean that an error handler will be invoked that can "smooth over" any possible errors:
  /// this checks solely if it will encode from code points into code units cleanly.

  /// @brief Validates the code points of the `input` according to the `encoding` with the given states @p
  /// encodeState and `decodeState`.
  /// @param[in] input The input range of code points to validate is possible for encoding into code units.
  /// @param[in] encoding The encoding to verify can properly encode the input of code units.
  /// @param[in] encodeState The state to use for the encoding portion of the validation check.
  /// @param[in] decodeState The state to use for the decoding portion of the validation check, if needed.
  /// @remarks This function explicitly does not check any of the extension points. It defers to doing a
  /// typical loop over the code points to verify it can be encoded into code units, and then decoded into
  /// code points, with no errors.
  template <typename TInput, typename TEncoding, typename TEncodeState, typename TDecodeState>
  constexpr auto BasicValidateEncodableAs(TInput &&input, TEncoding &&encoding, TEncodeState &encodeState,
                                          TDecodeState &decodeState)
  {
    using TWorkingInput = Krys::Ranges::subrange_for_t<Impl::span_reconstruct_t<TInput, TInput>>;
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TResult = ValidatePivotlessTranscodeResult<TWorkingInput, TEncodeState, TDecodeState>;

    TWorkingInput workingInput = Impl::SpanReconstruct<TInput>(std::forward<TInput>(input));

    if constexpr (Impl::HasTextValidateEncodableAsOne<TWorkingInput, TEncoding, TEncodeState, TDecodeState>)
    {
      for (;;)
      {
        auto result = TextValidateEncodableAsOne(Krys::Tag<TUEncoding> {}, std::move(workingInput),
                                                       encoding, encodeState, decodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, encodeState, decodeState);
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
      return TResult(std::move(workingInput), true, encodeState, decodeState);
    }
    else if constexpr (Impl::HasTextValidateEncodableAsOne<TWorkingInput, TEncoding, TEncodeState>)
    {
      for (;;)
      {
        auto result = TextValidateEncodableAsOne(Krys::Tag<TUEncoding> {}, std::move(workingInput),
                                                       encoding, encodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, encodeState, decodeState);
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
      return TResult(std::move(workingInput), true, encodeState, decodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateEncodableAsOne<TWorkingInput, TEncoding, TEncodeState,
                                                                   TDecodeState>)
    {
      for (;;)
      {
        auto result = TextValidateEncodableAsOne(Krys::Tag<TUEncoding> {}, std::move(workingInput),
                                                       encoding, encodeState, decodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, encodeState, decodeState);
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
      return TResult(std::move(workingInput), true, encodeState, decodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateEncodableAsOne<TWorkingInput, TEncoding, TEncodeState>)
    {
      for (;;)
      {
        auto result = InternalTextValidateEncodableAsOne(
          Krys::Tag<TUEncoding> {}, std::move(workingInput), encoding, encodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, encodeState, decodeState);
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
      return TResult(std::move(workingInput), true, encodeState, decodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateEncodableAsOne<TWorkingInput, TEncoding, TEncodeState>)
    {
      for (;;)
      {
        auto result = InternalTextValidateEncodableAsOne(
          Krys::Tag<TUEncoding> {}, std::move(workingInput), encoding, encodeState);
        if (!result.Valid)
        {
          return TResult(std::move(result.Input), false, encodeState, decodeState);
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
      return TResult(std::move(workingInput), true, encodeState, decodeState);
    }
    else
    {
      constexpr std::size_t codePointMax = MaxRecodeCodePoints<TUEncoding, TUEncoding>;
      constexpr std::size_t codeUnitMax = MaxCodeUnits<TUEncoding>;
      using TCodeUnit = code_unit_t<TUEncoding>;
      using TCodePoint = code_point_t<TUEncoding>;
      using TPivot = Span<TCodeUnit, codeUnitMax>;

      TCodePoint codePointBuf[codePointMax] {};
      TCodeUnit codeUnitBuf[codeUnitMax] {};
      Span<TCodePoint, codePointMax> codePointView(codePointBuf);
      TPivot pivot(codeUnitBuf);

      for (;;)
      {
        auto result = Krys::RecodeOneIntoRaw(std::move(workingInput), encoding, codePointView, encoding,
                                             Handlers::Pass, Handlers::Pass, encodeState, decodeState, pivot);
        if (result.ErrorCode != EncodingError::OK)
        {
          return TResult(std::move(result.Input), false, encodeState, decodeState);
        }
        const bool isRecodeRoundtripOK = Krys::Ranges::equal(
          Krys::Ranges::cbegin(workingInput), Krys::Ranges::cbegin(result.Input), codePointView.data(),
          std::to_address(Krys::Ranges::cbegin(result.Output)), Krys::equal_to_char);
        if (!isRecodeRoundtripOK)
        {
          return TResult(std::move(result.Input), false, encodeState, decodeState);
        }
        workingInput = std::move(result.Input);
        if (Krys::Ranges::empty(workingInput))
        {
          if (!Krys::IsStateComplete(encoding, encodeState))
          {
            continue;
          }
          if (!Krys::IsStateComplete(encoding, decodeState))
          {
            continue;
          }
          break;
        }
      }
      return TResult(std::move(workingInput), true, encodeState, decodeState);
    }
  }

  /// @brief Validates the code points of the `input` according to the `encoding` with the given states @p
  /// encodeState and `decodeState`.
  ///
  /// @param[in] input The input range of code points to validate is possible for encoding into code units.
  /// @param[in] encoding The encoding to verify can properly encode the input of code units.
  /// @param[in] encodeState The state to use for the encoding portion of the validation check.
  /// @param[in] decodeState The state to use for the decoding portion of the validation check, if needed.
  ///
  /// @remarks This functions checks to see if extension points for `TextValidateEncodableAs` is available
  /// taking the available 4 parameters. If so, it calls this. Otherwise, it defers to doing a typical loop
  /// over the code points to verify it can be encoded into code units, and then decoded into code points,
  /// with no errors.
  template <typename TInput, typename TEncoding, typename TEncodeState, typename TDecodeState>
  constexpr auto ValidateEncodableAs(TInput &&input, TEncoding &&encoding, TEncodeState &encodeState,
                                     TDecodeState &decodeState)
  {
    if constexpr (Impl::HasTextValidateEncodableAs<TInput, TEncoding, TEncodeState, TDecodeState>)
    {
      return TextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                           std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                           encodeState, decodeState);
    }
    else if constexpr (Impl::HasTextValidateEncodableAs<TInput, TEncoding, TEncodeState>)
    {
      (void)decodeState;
      return TextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                           std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                           encodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateEncodableAs<TInput, TEncoding, TEncodeState>)
    {
      (void)decodeState;
      return InternalTextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                                   std::forward<TInput>(input),
                                                   std::forward<TEncoding>(encoding), encodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateEncodableAs<TInput, TEncoding, TEncodeState,
                                                                TDecodeState>)
    {
      return InternalTextValidateEncodableAs(
        Krys::Tag<remove_cvref_t<TEncoding>> {}, std::forward<TInput>(input),
        std::forward<TEncoding>(encoding), encodeState, decodeState);
    }
    else
    {
      return Krys::BasicValidateEncodableAs(std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                            encodeState, decodeState);
    }
  }

  /// @brief Validates the code points of the `input` according to the `encoding` with the given states @p
  /// "encodeState".
  ///
  /// @param[in] input The input range of code points to validate is possible for encoding into code units.
  /// @param[in] encoding The encoding to verify can properly encode the input of code units.
  /// @param[in] encodeState The state for encoding to use.
  ///
  /// @remarks This functions checks to see if extension points for `TextValidateEncodableAs` is available
  /// taking the available 3 parameters. If so, it calls this. Otherwise, it defers to ValidateEncodableAs.
  template <typename TInput, typename TEncoding, typename TEncodeState>
  constexpr auto ValidateEncodableAs(TInput &&input, TEncoding &&encoding, TEncodeState &encodeState)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    if constexpr (Impl::HasTextValidateEncodableAs<TInput, TEncoding, TEncodeState>)
    {
      return TextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                           std::forward<TInput>(input), std::forward<TEncoding>(encoding),
                                           encodeState);
    }
    else if constexpr (Impl::HasInternalTextValidateEncodableAs<TInput, TEncoding, TEncodeState>)
    {
      return InternalTextValidateEncodableAs(Krys::Tag<remove_cvref_t<TEncoding>> {},
                                                   std::forward<TInput>(input),
                                                   std::forward<TEncoding>(encoding), encodeState);
    }
    else
    {
      using TState = decode_state_t<TUEncoding>;

      TState decodeState = Krys::CreateDecodeStateWith(encoding, encodeState);
      auto statefulResult = Krys::ValidateEncodableAs(std::forward<TInput>(input),
                                                std::forward<TEncoding>(encoding), encodeState, decodeState);
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

  /// @brief Validates the code points of the `input` according to the @p "encoding".
  ///
  /// @param[in] input The input range of code points to validate is possible for encoding into code units.
  /// @param[in] encoding The encoding to verify can properly encode the input of code units.
  template <typename TInput, typename TEncoding>
  constexpr auto ValidateEncodableAs(TInput &&input, TEncoding &&encoding)
  {
    using TUEncoding = remove_cvref_t<TEncoding>;
    using TState = encode_state_t<TUEncoding>;

    TState state = Krys::CreateEncodeState(encoding);
    auto statefulResult =
      Krys::ValidateEncodableAs(std::forward<TInput>(input), std::forward<TEncoding>(encoding), state);
    return Impl::SliceToStateless(std::move(statefulResult));
  }

  /// @brief Validates the code points of the input.
  ///
  /// @param[in] input The input range of code points to validate is possible for encoding into code units.
  ///
  /// @remarks This passes the default encoding as inferred from the discernible `value_type` of the input
  /// range input into the DefaultCodePointEncoding.
  template <typename TInput>
  constexpr auto ValidateEncodableAs(TInput &&input)
  {
    using TUInput = remove_cvref_t<TInput>;
    using TCodePoint = remove_cvref_t<Krys::Ranges::range_value_type_t<TUInput>>;
    if (std::is_constant_evaluated())
    {
      // Use literal encoding instead, if we meet the right criteria
      using TEncoding = default_consteval_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::ValidateEncodableAs(std::forward<TInput>(input), encoding);
    }
    else
    {
      using TEncoding = default_code_point_encoding_t<TCodePoint>;
      TEncoding encoding {};
      return Krys::ValidateEncodableAs(std::forward<TInput>(input), encoding);
    }
  }
}
