#pragma once

#include "Krystal.Text/BasicEncodingScheme.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"

namespace Krys
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
}
