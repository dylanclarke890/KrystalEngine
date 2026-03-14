#pragma once

#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include "Krystal.Text/Decode/DecodeResult.hpp"
#include "Krystal.Text/Encode/EncodeResult.hpp"

namespace Krys::Text
{
  /// @brief Whether or not the given `TEncoding` has a function called `SkipInputError` that takes the
  /// given `TResult` type with the given `TInputProgress` and `TOutputProgress` types.
  /// @tparam TEncoding The encoding that may contain the SkipInputError function.
  /// @tparam TResult The result type to check if the input is callable.
  /// @tparam TInputProgress The input progress type passed in to the error handler to be forwarded to the
  /// skip input error.
  /// @tparam TOutputProgress The output progress type passed in to the error handler to be forwarded to the
  /// skip input error.
  /// @remarks Used by ReplacementHandler and SkipHandler to pass over malformed input when it happens.
  template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
  concept HasSkipInputError = requires {
    std::declval<TEncoding>().SkipInputError(std::declval<TResult>(), std::declval<TInputProgress>(),
                                             std::declval<TOutputProgress>());
  };
}

namespace Krys::Text::detail
{
  /// @brief Checks whether calling SkipInputError is an exceptionless operation.
  template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
  consteval bool IsSkipInputErrorNoexcept() noexcept
  {
    if constexpr (HasSkipInputError<const TEncoding &, TResult, const TInputProgress &,
                                    const TOutputProgress &>)
    {
      return noexcept(std::declval<TEncoding>().SkipInputError(
        std::declval<TResult>(), std::declval<TInputProgress>(), std::declval<TOutputProgress>()));
    }
    else
    {
      return true;
    }
  }
}

namespace Krys::Text
{
  /// @brief Checks whether calling SkipInputError is an exceptionless operation.
  template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
  concept NoThrowSkipInputError =
    ::Krys::Text::detail::IsSkipInputErrorNoexcept<const TEncoding &, TResult, const TInputProgress &,
                                                   const TOutputProgress &>();

  /// @brief Attempts to skip over an input error in the text.
  /// @param[in] result The current result state of the encode or decode operation.
  /// @param[in] inputProgress A contiguous range containing all of the (potentially) irreversibly read
  /// input from an encoding operation.
  /// @param[in] outputProgress A contiguous range containing all of the (potentially) irreversibly
  /// written output from an encoding operation.
  /// @remarks This function is specifically for UTF-32 input, where e.g. multiple surrogates may be part of
  /// the incoming text and the target encoding does not support that. Therefore, it will skip over every
  /// too-large codepoint, and every surrogate pair codepoint, before stopping.
  template <typename TResult, typename TInputProgress, typename TOutputProgress>
  constexpr auto SkipUTF32InputError(TResult &&result, const TInputProgress &inputProgress,
                                     const TOutputProgress &outputProgress) noexcept
  {
    // we can only advance one character at a time.
    auto it = std::ranges::begin(std::forward<TResult>(result).Input);
    auto last = std::ranges::end(result.Input);

    using TInput = decltype(result.Input);
    using TUInput = remove_cvref_t<TInput>;
    using TOutput = decltype(result.Output);
    using TState = remove_ref_t<unwrap_t<remove_cvref_t<decltype(result.State)>>>;
    using TReconstructedInput = ::Krys::Ranges::reconstruct_t<TUInput, decltype(it) &&, decltype(last) &&>;
    using TResultType = conditional_t<::Krys::IsSpecializationOf<TResult, DecodeResult>,
                                      DecodeResult<TReconstructedInput, TOutput, TState>,
                                      EncodeResult<TReconstructedInput, TOutput, TState>>;
    if (it != last)
    {
      // if there is already some items in the input progress (things irreversibly read), then
      // we are not obligated to do "at least" one skip; barrier it behind an empty check for
      // progress.
      if (std::ranges::empty(inputProgress) && std::ranges::empty(outputProgress))
      {
        ++it;
      }
      for (; it != last; ++it)
      {
        char32 ch32 = static_cast<char32>(*it);
        if (ch32 < ::Krys::Text::Unicode::LastUnicodeCodePoint && !::Krys::Text::Unicode::IsSurrogate(ch32))
        {
          break;
        }
      }
    }
    return TResult(::Krys::Ranges::reconstruct(std::in_place_type<TUInput>, std::move(it), std::move(last)),
                   std::move(result.Output), result.State, result.ErrorCode, result.ErrorCount);
  }

  /// @brief Attempts to skip over an input error in the text.
  /// @param[in] result The current result state of the encode or decode operation.
  /// @param[in] inputProgress A contiguous range containing all of the (potentially) irreversibly read
  /// input from an encoding operation.
  /// @param[in] outputProgress A contiguous range containing all of the (potentially) irreversibly
  /// written output from an encoding operation.
  /// @remarks This function is specifically for UTF-32 input that also includes surrogate values as a valid
  /// option. Therefore, it will skip over every too-large codepoint.
  template <typename TResult, typename TInputProgress, typename TOutputProgress>
  constexpr auto SkipUTF32WithSurrogatesInputError(TResult &&result, const TInputProgress &inputProgress,
                                                   const TOutputProgress &outputProgress) noexcept
  {
    // we can only advance one character at a time.
    auto it = std::ranges::begin(std::forward<TResult>(result).Input);
    auto last = std::ranges::end(result.Input);

    using TInput = decltype(result.Input);
    using TUInput = remove_cvref_t<TInput>;
    using TOutput = decltype(result.Output);
    using TState = remove_ref_t<unwrap_t<remove_cvref_t<decltype(result.State)>>>;
    using TReconstructedInput = ::Krys::Ranges::reconstruct_t<TUInput, decltype(it) &&, decltype(last) &&>;
    using TResultType = conditional_t<::Krys::IsSpecializationOf<remove_cvref_t<TResult>, DecodeResult>,
                                      DecodeResult<TReconstructedInput, TOutput, TState>,
                                      EncodeResult<TReconstructedInput, TOutput, TState>>;

    if (it != last)
    {
      // if there is already some items in the input progress (things irreversibly read), then
      // we are not obligated to do "at least" one skip; barrier it behind an empty check for
      // progress.
      if (std::ranges::empty(inputProgress) && std::ranges::empty(outputProgress))
      {
        ++it;
      }
      for (; it != last; ++it)
      {
        if (static_cast<char32>(*it) < ::Krys::Text::Unicode::LastUnicodeCodePoint)
        {
          break;
        }
      }
    }
    return TResult(::Krys::Ranges::reconstruct(std::in_place_type<TUInput>, std::move(it), std::move(last)),
                   std::move(result.Output), result.State, result.ErrorCode, result.ErrorCount);
  }

  /// @brief Attempts to skip over an input error in the text.
  /// @param[in] encoding The Encoding that experienced the error.
  /// @param[in] result The current result state of the encode or decode operation.
  /// @param[in] inputProgress A contiguous range containing all of the (potentially) irreversibly read
  /// input from an encoding operation.
  /// @param[in] outputProgress A contiguous range containing all of the (potentially) irreversibly
  /// written output from an encoding operation.
  /// @remarks If there exists a well-formed function call of the form
  /// `encoding.SkipInputError(result)`, it will call that function. Otherwise, it will attempt to
  /// grab the input iterator and pre-increment it exactly once. The goal for this is to provide
  /// functionality which can smartly skip over a collection of ill-formed code units or bytes in an input
  /// sequence, rather than generated e.g. 3 different replacement characters for a mal-formed UTF-8
  /// sequence. For example, given this malformed wineglass code point as an input UTF-8 sequence:
  /// `"\xC0\x9F\x8D\xB7meow"` when used in conjunction with utf8 (and similar), a proper decode/transcode
  /// call will error on `'\\xC0'`. Then, this function skips until the `'m'` input code unit, resulting in a
  /// leftover sequence of "meow".
  template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
  constexpr auto SkipInputError(
    const TEncoding &encoding, TResult &&result, const TInputProgress &inputProgress,
    const TOutputProgress
      &outputProgress) noexcept(NoThrowSkipInputError<const TEncoding &, TResult, const TInputProgress &,
                                                      const TOutputProgress &>)
  {
    if constexpr (HasSkipInputError<const TEncoding &, TResult, const TInputProgress &,
                                    const TOutputProgress &>)
    {
      return encoding.SkipInputError(std::forward<TResult>(result), inputProgress, outputProgress);
    }
    else
    {
      auto input = std::forward<TResult>(result).Input;
      auto it = std::ranges::begin(input);
      auto last = std::ranges::end(input);

      if (it != last)
      {
        if (std::ranges::empty(inputProgress) && std::ranges::empty(outputProgress))
        {
          ++it;
        }
      }

      result.Input = ::Krys::Ranges::reconstruct(std::in_place_type<remove_cvref_t<decltype(result.Input)>>,
                                                 std::move(it), std::move(last));

      return std::forward<TResult>(result);
    }
  }
}
