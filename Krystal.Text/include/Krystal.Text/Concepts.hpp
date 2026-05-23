#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"

namespace Krys::Text::detail_concepts
{
  template <typename T>
  concept HasIsInjective = requires { typename T::is_injective; };

  template <typename T>
  concept HasIsEncodeInjective = requires { typename T::is_encode_injective; };

  template <typename T>
  concept HasIsDecodeInjective = requires { typename T::is_decode_injective; };
}

namespace Krys::Text
{
  /// @brief Whether or not the decode encoding is redundant.
  template <typename TFrom, typename TTo>
  concept IsDecodeRedundant = SameType<remove_cvref_t<TFrom>, remove_cvref_t<TTo>>;

  /// @brief Whether or not the decode encoding is redundant.
  template <typename TFrom, typename TTo>
  concept IsEncodeRedundant = SameType<remove_cvref_t<TFrom>, remove_cvref_t<TTo>>;

  /// @brief Whether or not the given `T` is an error handler that can be ignored.
  template <typename T>
  concept IsIgnorableErrorHandler = remove_cvref_t<T>::assume_valid::value == true;

  /// @brief Checks whether the given encoding type returns a maybe-replacement range of code points.
  template <typename T, typename... TArgs>
  concept HasMaybeReplacementCodePoints =
    requires { std::declval<const T &>().MaybeReplacementCodePoints(std::declval<TArgs>()...); };

  /// @brief Checks whether the given encoding type returns a maybe-replacement range of code points.
  template <typename T, typename... TArgs>
  concept HasReplacementCodePoints =
    requires { std::declval<const T &>().ReplacementCodePoints(std::declval<TArgs>()...); };

  /// @brief Checks whether the given encoding type returns a maybe-replacement range of code units.
  /// @tparam T The type to check.
  /// @remarks The `value` boolean is true if the given `T` has a function named @c
  /// MaybeReplacementCodeUnits() on it that can be called from a `const` -qualified `T` which returns
  /// a `std::optional` containing a contiguous view of code units.
  template <typename T, typename... TArgs>
  concept HasMaybeReplacementCodeUnits =
    requires { std::declval<const T &>().MaybeReplacementCodeUnits(std::declval<TArgs>()...); };

  /// @brief Checks whether the given encoding type returns a replacement range of code units.
  /// @tparam T The type to check.
  /// @remarks The `value` boolean is true if the given `T` has a function named
  /// `ReplacementCodeUnits()` on it that can be called from a `const`-qualified `T` which returns a
  /// contiguous view of code units.
  template <typename T, typename... TArgs>
  concept HasReplacementCodeUnits =
    requires { std::declval<const T &>().ReplacementCodeUnits(std::declval<TArgs>()...); };

  /// @brief Checks whether or not the decoding step for `T` is injective (cannot possibly lose
  /// information regardless of whatever valid input is put in).
  /// @tparam T The encoding type to check.
  template <typename T>
  concept IsDecodeInjective = ::Krys::Text::detail_concepts::HasIsDecodeInjective<remove_cvref_t<T>>
                              || ::Krys::Text::detail_concepts::HasIsInjective<remove_cvref_t<T>>;

  /// @brief Checks whether or not the encoding step for `T` is injective (cannot possibly lose
  /// information regardless of whatever valid input is put in).
  /// @tparam T The encoding type to check.
  template <typename T>
  concept IsEncodeInjective = ::Krys::Text::detail_concepts::HasIsEncodeInjective<remove_cvref_t<T>>
                              || ::Krys::Text::detail_concepts::HasIsInjective<remove_cvref_t<T>>;

  template <typename TInput, typename TEncoding, typename TOutput, typename THandler, typename TState>
  concept HasDecodeText = requires {
    DecodeText(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(), std::declval<TEncoding>(),
               std::declval<TOutput>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename TOutput, typename THandler, typename TState>
  concept HasEncodeText = requires {
    EncodeText(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(), std::declval<TEncoding>(),
               std::declval<TOutput>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TFromEncoding, typename TOutput, typename TToEncoding,
            typename TFromHandler, typename TToHandler, typename TFromState, typename TToState,
            typename TPivot>
  concept HasTranscodeText = requires {
    TranscodeText(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>>(),
                  std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TOutput>(),
                  std::declval<TToEncoding>(), std::declval<TFromHandler>(), std::declval<TToHandler>(),
                  std::declval<TFromState &>(), std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasCountTextAsDecoded = requires {
    CountTextAsDecoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                       std::declval<TEncoding>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasCountTextAsDecodedOne = requires {
    CountTextAsDecodedOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                          std::declval<TEncoding>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasCountTextAsEncoded = requires {
    CountTextAsEncoded(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TInput>(),
                       std::declval<TEncoding>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TEncoding, typename THandler, typename TState>
  concept HasCountTextAsEncodedOne = requires {
    CountTextAsEncodedOne(Krys::Tag<remove_cvref_t<TEncoding>> {}, std::declval<TEncoding>(),
                          std::declval<TInput>(), std::declval<THandler>(), std::declval<TState &>());
  };

  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromHandler,
            typename TToHandler, typename TFromState, typename TToState, typename TPivot>
  concept HasCountTextAsTranscoded = requires {
    CountTextAsTranscoded(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                          std::declval<TInput>(), std::declval<TFromEncoding>(), std::declval<TToEncoding>(),
                          std::declval<TFromHandler>(), std::declval<TToHandler>(),
                          std::declval<TFromState &>(), std::declval<TToState &>(), std::declval<TPivot>());
  };

  template <typename TInput, typename TFromEncoding, typename TToEncoding, typename TFromHandler,
            typename TToHandler, typename TFromState, typename TToState, typename TPivot>
  concept HasCountTextAsTranscodedOne = requires {
    CountTextAsTranscodedOne(Krys::Tag<remove_cvref_t<TFromEncoding>, remove_cvref_t<TToEncoding>> {},
                             std::declval<TInput>(), std::declval<TFromEncoding>(),
                             std::declval<TToEncoding>(), std::declval<TFromHandler>(),
                             std::declval<TToHandler>(), std::declval<TFromState &>(),
                             std::declval<TToState &>(), std::declval<TPivot>());
  };
}