#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/IsTranscodingCompatible.hpp"

namespace Krys
{
  namespace Impl
  {
    template <typename T>
    concept HasIsInjective = requires { T::is_injective::value; };

    template <typename T>
    concept HasIsEncodeInjective = requires { T::is_encode_injective::value; };

    template <typename T>
    concept HasIsDecodeInjective = requires { T::is_decode_injective::value; };
  }

  /// @brief Checks whether or not the decoding step for `T` is injective (cannot possibly lose
  /// information regardless of whatever valid input is put in).
  /// @tparam T The encoding type to check.
  /// @remarks If the encoding object does not define IsDecodeInjective, it is assumed to be false (the
  /// safest default).
  template <typename T>
  concept IsDecodeInjective =
    Impl::HasIsDecodeInjective<remove_cvref_t<T>> || Impl::HasIsInjective<remove_cvref_t<T>>;

  /// @brief Checks whether or not the encoding step for `T` is injective (cannot possibly lose
  /// information regardless of whatever valid input is put in).
  /// @tparam T The encoding type to check.
  /// @remarks If the encoding object does not define IsEncodeInjective, it is assumed to be false (the
  /// safest default).
  template <typename T>
  concept IsEncodeInjective =
    Impl::HasIsEncodeInjective<remove_cvref_t<T>> || Impl::HasIsInjective<remove_cvref_t<T>>;

  /// @brief Checks whether a decode operation with `TFrom` piped to a decode operation with `TTo` is
  template <typename TFrom, typename TTo>
  concept IsFullRangeRepresentableBetween =
    IsDecodeInjective<remove_cvref_t<TFrom>> && IsEncodeInjective<remove_cvref_t<TTo>>;

  template <typename T>
  concept IsFullRangeRepresentable = IsFullRangeRepresentableBetween<T, T>;
}
