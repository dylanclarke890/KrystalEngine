#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/EncodingName.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"
#include "Krystal.Text/EncodingId.hpp"
#include <type_traits>

namespace Krys::Text::detail
{
  template <typename, typename = void>
  struct unicode_encoding : std::false_type
  {
  };

  template <typename T>
  concept HasIsUnicodeEncoding = requires { typename T::is_unicode_encoding; };

  template <typename T>
  struct unicode_encoding<T, enable_if_t<HasIsUnicodeEncoding<T>>>
      : IntegralConstant<bool, T::is_unicode_encoding::value>
  {
  };

  template <typename T>
  consteval EncodingId GetDecodeId() noexcept
  {
    if constexpr (requires { T::DecodedId; })
    {
      return T::DecodedId;
    }
    else if constexpr (IsUnicodeCodePoint<code_point_t<T>> && unicode_encoding<remove_cvref_t<T>>::value)
    {
      return EncodingId::utf32;
    }
    else
    {
      return EncodingId::unknown;
    }
  }

  template <typename T>
  consteval EncodingId GetEncodeId() noexcept
  {
    if constexpr (requires { T::EncodedId; })
    {
      return T::EncodedId;
    }
    else if constexpr (IsUnicodeCodePoint<code_unit_t<T>> && unicode_encoding<remove_cvref_t<T>>::value)
    {
      return EncodingId::utf32;
    }
    else
    {
      return EncodingId::unknown;
    }
  }
}

namespace Krys::Text
{
  /// @brief Checks whether or not the encoding has declared it can handle all of Unicode.
  /// @tparam T The encoding type to check.
  /// @remarks If the encoding object does not define is_unicode_encoding, it is assumed to be false (the
  /// safest default).
  template <typename T>
  concept UnicodeEncoding = ::Krys::Text::detail::unicode_encoding<remove_cvref_t<T>>::value;

  /// @brief Returns the ID of what an encoding decodes into.
  /// @tparam T The encoding type to retrieve the ID from.
  /// @remarks If the encoding type does not have a `constexpr static EncodingId` member with the name
  /// `DecodedId`, it will assume it decodes to UTF-32 code points if the code_point_t type
  /// matches IsUnicodeCodePoint. Otherwise, it will return EncodingId::unknown.
  template <typename T>
  constexpr inline EncodingId DecodeId = ::Krys::Text::detail::GetDecodeId<T>();

  /// @brief Returns the ID of what an encoding encodes into.
  /// @tparam T The encoding type to retrieve the ID from.
  /// @remarks If the encoding type does not have a `constexpr static EncodingId` member with the name
  /// `EncodedId`, it will assume it decodes to UTF-32 code points if the code_unit_t type
  /// matches IsUnicodeCodePoint. Otherwise, it will return EncodingId::unknown.
  template <typename T>
  constexpr inline EncodingId EncodeId = ::Krys::Text::detail::GetEncodeId<T>();
}