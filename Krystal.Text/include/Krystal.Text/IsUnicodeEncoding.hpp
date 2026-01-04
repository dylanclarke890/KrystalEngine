#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/EncodingName.hpp"
#include "Krystal.Text/IsFullRangeRepresentable.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/TextEncodingId.hpp"
#include <type_traits>

namespace Krys
{
  namespace Impl
  {
    template <typename T>
    concept HasContainsUnicodeEncoding = requires { std::declval<const T &>().ContainsUnicodeEncoding(); };

    template <typename T>
    concept HasIsUnicodeEncoding = T::is_unicode_encoding::value;

    template <typename, typename = void>
    struct IsUnicodeEncodingSfinae : std::false_type
    {
    };

    template <typename T>
    struct IsUnicodeEncodingSfinae<T, enable_if_t<HasIsUnicodeEncoding<T>>>
        : IntegralConstant<bool, T::is_unicode_encoding::value>
    {
    };
  }

  /// @brief Checks whether or not the encoding has declared it can handle all of Unicode.
  /// @tparam T The encoding type to check.
  /// @remarks If the encoding object does not define IsUnicodeEncoding, it is assumed to be false (the
  /// safest default).
  template <typename T>
  concept IsUnicodeEncoding = Impl::IsUnicodeEncodingSfinae<remove_cvref_t<T>>::value;

  /// @brief Whether or not the provided encoding is a Unicode encoding.
  /// @param[in] encoding The encoding to query.
  /// @remarks This function first checks if there is a function called `ContainsUnicodeEncoding` . If it
  /// is present, then it returns the value of that function directly. Otherwise, it checks if
  /// IsUnicodeEncoding is true for the provided `encoding` . If that's the case, then
  /// `true` is returned. Otherwise, it assumes the encoding is not a Unicode-compatible encoding and
  /// returns false.
  template <typename TEncoding>
  constexpr bool ContainsUnicodeEncoding(const TEncoding &encoding) noexcept
  {
    if constexpr (Impl::HasContainsUnicodeEncoding<TEncoding>)
    {
      return encoding.ContainsUnicodeEncoding();
    }
    else if constexpr (IsUnicodeEncoding<TEncoding>)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  namespace Impl
  {
    template <typename T>
    concept HasDecodedId = requires { T::DecodedId; };

    template <typename T, typename = void>
    struct DecodedIdSfinae
        : IntegralConstant<TextEncodingId, IsUnicodeCodePoint<code_point_t<T>> && IsUnicodeEncoding<T>
                                             ? TextEncodingId::utf32
                                             : TextEncodingId::unknown>
    {
    };

    template <typename T>
    struct DecodedIdSfinae<T, enable_if_t<HasDecodedId<T>>> : IntegralConstant<TextEncodingId, T::DecodedId>
    {
    };

    template <typename T>
    concept HasEncodedId = requires { T::EncodedId; };

    template <typename T, typename = void>
    struct EncodedIdSfinae
        : IntegralConstant<TextEncodingId, IsUnicodeCodePoint<code_unit_t<T>> && IsUnicodeEncoding<T>
                                             ? TextEncodingId::utf32
                                             : TextEncodingId::unknown>
    {
    };

    template <typename T>
    struct EncodedIdSfinae<T, enable_if_t<HasEncodedId<T>>> : IntegralConstant<TextEncodingId, T::EncodedId>
    {
    };
  }

  /// @brief Returns the ID of what an encoding decodes into.
  /// @tparam T The encoding type to retrieve the ID from.
  /// @remarks If the encoding type does not have a `static constexpr TextEncodingId` member with the name
  /// `DecodedId`, it will assume it decodes to UTF-32 code points if the code_point_t type
  /// matches IsUnicodeCodePoint. Otherwise, it will return TextEncodingId::unknown.
  template <typename T>
  class DecodedId : public Impl::DecodedIdSfinae<T>
  {
  };

  /// @brief An alias of the inner `value` for IsUnicodeEncoding.
  template <typename T>
  constexpr inline TextEncodingId decoded_id_v = DecodedId<T>::value;

  /// @brief Returns the ID of what an encoding encodes into.
  /// @tparam T The encoding type to retrieve the ID from.
  /// @remarks If the encoding type does not have a `static constexpr TextEncodingId` member with the name
  /// `EncodedId`, it will assume it decodes to UTF-32 code points if the code_unit_t type
  /// matches IsUnicodeCodePoint. Otherwise, it will return TextEncodingId::unknown.
  template <typename T>
  class EncodedId : public Impl::EncodedIdSfinae<remove_cvref_t<T>>
  {
  };

  /// @brief An alias of the inner `value` for IsUnicodeEncoding.
  template <typename T>
  constexpr inline TextEncodingId encoded_id_v = EncodedId<remove_cvref_t<T>>::value;
}