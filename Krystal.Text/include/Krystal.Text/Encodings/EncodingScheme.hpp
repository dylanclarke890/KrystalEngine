#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/EncodingName.hpp"
#include "Krystal.Text/Encodings/ASCII.hpp"
#include "Krystal.Text/Encodings/BasicEncodingScheme.hpp"
#include "Krystal.Text/Encodings/EncodingScheme.hpp"
#include "Krystal.Text/Encodings/NoEncoding.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/EncodingId.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <string_view>

namespace Krys::Text
{
  /// @brief A UTF-16 encoding, in Little Endian format, with inputs as a sequence of bytes.
  ///
  /// @tparam TByte The byte type to use. Typically, this is `std::byte` or `uchar.`
  template <typename TByte>
  using basic_utf16_le = EncodingScheme<utf16_t, Endian::Type::Little, TByte>;

  /// @brief A UTF-16 encoding, in Big Endian format, with inputs as a sequence of bytes.
  ///
  /// @tparam TByte The byte type to use. Typically, this is `std::byte` or <tt>uchar</tt>.
  template <typename TByte>
  using basic_utf16_be = EncodingScheme<utf16_t, Endian::Type::Big, TByte>;

  /// @brief A UTF-16 encoding, in Native Endian format, with inputs as a sequence of bytes.
  ///
  /// @tparam TByte The byte type to use. Typically, this is `std::byte` or <tt>uchar</tt>.
  template <typename TByte>
  using basic_utf16_ne = EncodingScheme<utf16_t, Endian::Type::System, TByte>;

  /// @brief A UTF-16 encoding, in Little Endian format, with inputs as a sequence of bytes.
  using utf16_le_t = basic_utf16_le<std::byte>;

  /// @brief A UTF-16 encoding, in Big Endian format, with inputs as a sequence of bytes.
  using utf16_be_t = basic_utf16_be<std::byte>;

  /// @brief A UTF-16 encoding, in Native Endian format, with inputs as a sequence of bytes.
  using utf16_ne_t = basic_utf16_ne<std::byte>;

  /// @brief A UTF-32 encoding, in Little Endian format, with inputs as a sequence of bytes.
  ///
  /// @tparam TByte The byte type to use. Typically, this is `std::byte` or <tt>uchar</tt> .
  template <typename TByte>
  using basic_utf32_le = EncodingScheme<utf32_t, Endian::Type::Little, TByte>;

  /// @brief A UTF-32 encoding, in Big Endian format, with inputs as a sequence of bytes.
  ///
  /// @tparam TByte The byte type to use. Typically, this is `std::byte` or <tt>uchar</tt> .
  template <typename TByte>
  using basic_utf32_be = EncodingScheme<utf32_t, Endian::Type::Big, TByte>;

  /// @brief A UTF-32 encoding, in Native Endian format, with inputs as a sequence of bytes.
  ///
  /// @tparam TByte The byte type to use. Typically, this is `std::byte` or <tt>uchar</tt> .
  template <typename TByte>
  using basic_utf32_ne = EncodingScheme<utf32_t, Endian::Type::System, TByte>;

  /// @brief A UTF-32 encoding, in Little Endian format, with inputs as a sequence of bytes.
  using utf32_le_t = basic_utf32_le<std::byte>;

  /// @brief A UTF-32 encoding, in Big Endian format, with inputs as a sequence of bytes.
  using utf32_be_t = basic_utf32_be<std::byte>;

  /// @brief A UTF-32 encoding, in Big Endian format, with inputs as a sequence of bytes.
  using utf32_ne_t = basic_utf32_ne<std::byte>;

  /// @brief An instance of the utf16_le_t type for ease of use.
  constexpr inline utf16_le_t utf16_le = {};

  /// @brief An instance of the utf16_be_t type for ease of use.
  constexpr inline utf16_be_t utf16_be = {};

  /// @brief An instance of the utf16_ne_t type for ease of use.
  constexpr inline utf16_ne_t utf16_ne = {};

  /// @brief An instance of the utf32_le_t type for ease of use.
  constexpr inline utf32_le_t utf32_le = {};

  /// @brief An instance of the utf32_ne_t type for ease of use.
  constexpr inline utf32_be_t utf32_be = {};

  /// @brief An instance of the utf32_ne_t type for ease of use.
  constexpr inline utf32_ne_t utf32_ne = {};

  template <typename TChar, EncodingId Id>
  KRYS_NODISCARD constexpr auto SelectCompileTimeEncoding()
  {
    if constexpr (Id == EncodingId::utf8)
    {
      return basic_utf8<TChar> {};
    }
    else if constexpr (Id == EncodingId::mutf8)
    {
      return basic_mutf8<TChar> {};
    }
    else if constexpr (Id == EncodingId::wtf8)
    {
      return basic_wtf8<TChar> {};
    }
    else if constexpr (Id == EncodingId::utf16)
    {
      return basic_utf16<TChar> {};
    }
    else if constexpr (Id == EncodingId::utf16le)
    {
      if constexpr (Endian::IsSystemLittleEndian())
      {
        return basic_utf16<TChar> {};
      }
      else
      {
        return basic_utf16_le<TChar> {};
      }
    }
    else if constexpr (Id == EncodingId::utf16be)
    {
      if constexpr (Endian::IsSystemBigEndian())
      {
        return basic_utf16<TChar> {};
      }
      else
      {
        return basic_utf16_be<TChar> {};
      }
    }
    else if constexpr (Id == EncodingId::utf32)
    {
      return basic_utf32<TChar> {};
    }
    else if constexpr (Id == EncodingId::utf32le)
    {
      if constexpr (Endian::IsSystemLittleEndian())
      {
        return basic_utf32<TChar> {};
      }
      else
      {
        return basic_utf32_be<TChar> {};
      }
    }
    else if constexpr (Id == EncodingId::utf32be)
    {
      if constexpr (Endian::IsSystemBigEndian())
      {
        return basic_utf32<TChar> {};
      }
      else
      {
        return basic_utf32_be<TChar> {};
      }
    }
    else if constexpr (Id == EncodingId::ascii)
    {
      return basic_ascii<TChar> {};
    }
    else
    {
      return basic_no_encoding<TChar, UnicodeCodePoint> {};
    }
  }
}
