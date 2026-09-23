#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"

namespace krys::text
{
#pragma region UTF8

  /// @brief The UTF-8 Encoding.
  struct UTF8 : public DefaultEncodingTraits
  {
    using code_unit = char8;

    constexpr static EncodingId Id = EncodingId::UTF8;
    constexpr static utf8_stringview Name = u8"UTF-8";
    constexpr static Array<utf8_stringview, 6uz> Aliases = {
      u8"utf-8", u8"utf8", u8"unicode-1-1-utf-8", u8"unicode11utf8", u8"unicode20utf8", u8"x-unicode20utf8"};

    constexpr static size_t MaxEncodeCodeUnits = 4uz;
    constexpr static bool IsEncodeLossless = true;
    constexpr static Array<code_unit, 3uz> ReplacementCodeUnits = {0xEF, 0xBF, 0xBD};
    constexpr static Array<code_unit, 3uz> BOM = {0xEF, 0xBB, 0xBF};

    /// @brief The first part of the table maps bytes to character classes to reduce the size of the
    /// transition table and create bitmasks. The second part is a transition table that maps a
    /// combination of a state of the automaton and a character class to a state.
    constexpr static Array<uint8, 364uz> LookupTable = {
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
      1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
      9,  9,  9,  9,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
      7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  10, 3,  3,  3,  3,  3,  3,  3,  3,  3,
      3,  3,  3,  4,  3,  3,  11, 6,  6,  6,  5,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
      8, // end of part one

      0,  12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 0,
      12, 12, 12, 12, 12, 0,  12, 0,  12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 24, 12, 12, 12, 12, 12, 12,
      12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12,
      12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 12,
      12, 12, 12, 12,
    };

    template <CodeUnitInputIterator<UTF8> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<UTF8> DecodeOne(TIterator current, TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      constexpr static uint8 Accept = 0u;
      constexpr static uint8 Reject = 12u;

      // Note: adapted from Bjoern Hoehrmann's DFA-based UTF-8 decoder
      // see: https://bjoern.hoehrmann.de/utf-8/decoder/dfa/

      uint32 state = Accept; // Start state must be 0
      uint32 codePoint = 0;
      uint8 consumed = 0u;

      while (current != end)
      {
        uint32 byte = static_cast<uint32>(*current);
        consumed++;
        current++;

        uint32 type = LookupTable[byte];
        codePoint = (state != Accept) ? (byte & 0x3FU) | (codePoint << 6) : (0xFF >> type) & (byte);
        state = LookupTable[256uz + state + type];

        if (state == Accept)
        {
          return DecodeOneSuccess<UTF8>(static_cast<char32>(codePoint), CodeUnitsConsumed(consumed));
        }
        else if (state == Reject)
        {
          return DecodeOneInvalidSequenceError<UTF8>(CodeUnitsConsumed(consumed));
        }

        // state is positive (e.g., 24, 36) -> we need more input, so loop continues
      }

      // If we exit the loop, we ran out of input before the DFA reached Accept
      return DecodeOneIncompleteSequenceError<UTF8>(CodeUnitsConsumed(consumed));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<UTF8> EncodeOne(char32 codePoint) noexcept
    {
      auto length = DecodeUTF8Length(codePoint);
      switch (length)
      {
        case 1uz:
        {
          return EncodeOneSuccess<UTF8>(static_cast<code_unit>(codePoint));
        }
        case 2uz:
        {
          return EncodeOneSuccess<UTF8>(static_cast<code_unit>(0xC0 | (codePoint >> 6)),
                                        static_cast<code_unit>(0x80 | (codePoint & 0x3F)));
        }
        case 3uz:
        {
          return EncodeOneSuccess<UTF8>(static_cast<code_unit>(0xE0 | (codePoint >> 12)),
                                        static_cast<code_unit>(0x80 | ((codePoint >> 6) & 0x3F)),
                                        static_cast<code_unit>(0x80 | (codePoint & 0x3F)));
        }
        case 4uz:
        {
          return EncodeOneSuccess<UTF8>(static_cast<code_unit>(0xF0 | (codePoint >> 18)),
                                        static_cast<code_unit>(0x80 | ((codePoint >> 12) & 0x3F)),
                                        static_cast<code_unit>(0x80 | ((codePoint >> 6) & 0x3F)),
                                        static_cast<code_unit>(0x80 | (codePoint & 0x3F)));
        }
      }

      return EncodeOneUnmappableCodePointError<UTF8>();
    }
  };

  static_assert(Encoding<UTF8>);

#pragma endregion

#pragma region UTF16

  /// @brief The UTF-16LE Encoding.
  struct UTF16LE : public DefaultEncodingTraits
  {
    using code_unit = char16;

    constexpr static EncodingId Id = EncodingId::UTF16LE;
    constexpr static utf8_stringview Name = u8"UTF-16LE";
    constexpr static Array<utf8_stringview, 2uz> Aliases = {u8"utf-16le", u8"csutf16le"};

    constexpr static size_t MaxEncodeCodeUnits = 2uz;
    constexpr static bool IsEncodeLossless = true;
    constexpr static Array<code_unit, 1uz> ReplacementCodeUnits = {0xFFFD};
    constexpr static Array<code_unit, 2uz> BOM = {0xFF, 0xFE};

    template <CodeUnitInputIterator<UTF16LE> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<UTF16LE> DecodeOne(TIterator current,
                                                                       TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      auto leadCodeUnit = Endian::ToSystemEndian<Endian::Little>(static_cast<char16>(*current));
      if (!IsSingleOrLeadUTF16(leadCodeUnit))
      {
        return DecodeOneInvalidSequenceError<UTF16LE>(CodeUnitsConsumed(1u));
      }

      if (IsSingleUTF16(leadCodeUnit))
      {
        return DecodeOneSuccess<UTF16LE>(leadCodeUnit, CodeUnitsConsumed(1u));
      }

      if (++current == end)
      {
        return DecodeOneIncompleteSequenceError<UTF16LE>(CodeUnitsConsumed(1u));
      }

      auto trailCodeUnit = Endian::ToSystemEndian<Endian::Little>(static_cast<char16>(*current));
      if (!IsTrailSurrogate(trailCodeUnit))
      {
        return DecodeOneInvalidSequenceError<UTF16LE>(CodeUnitsConsumed(2u));
      }

      return DecodeOneSuccess<UTF16LE>(UTF16CombineSurrogates(leadCodeUnit, trailCodeUnit),
                                       CodeUnitsConsumed(2u));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<UTF16LE> EncodeOne(char32 codePoint) noexcept
    {
      if (IsNonUnicodeCharacter(codePoint))
      {
        return EncodeOneUnmappableCodePointError<UTF16LE>();
      }

      if (codePoint < 0x10000)
      {
        return EncodeOneSuccess<UTF16LE>(Endian::ToLittleEndian(static_cast<char16>(codePoint)));
      }

      auto [lead, trail] = UTF16SplitSurrogates(codePoint);
      return EncodeOneSuccess<UTF16LE>(Endian::ToLittleEndian(lead), Endian::ToLittleEndian(trail));
    }
  };

  static_assert(Encoding<UTF16LE>);

  /// @brief The UTF-16BE Encoding.
  struct UTF16BE : public DefaultEncodingTraits
  {
    using code_unit = char16;

    constexpr static EncodingId Id = EncodingId::UTF16BE;
    constexpr static utf8_stringview Name = u8"UTF-16BE";
    constexpr static Array<utf8_stringview, 2uz> Aliases = {u8"utf-16be", u8"csutf16be"};

    constexpr static size_t MaxEncodeCodeUnits = 2uz;
    constexpr static bool IsEncodeLossless = true;
    constexpr static Array<code_unit, 1uz> ReplacementCodeUnits = {0xFFFD};
    constexpr static Array<code_unit, 2uz> BOM = {0xFE, 0xFF};

    template <CodeUnitInputIterator<UTF16BE> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<UTF16BE> DecodeOne(TIterator current,
                                                                       TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      auto leadCodeUnit = Endian::ToSystemEndian<Endian::Big>(static_cast<char16>(*current));
      if (!IsSingleOrLeadUTF16(leadCodeUnit))
      {
        return DecodeOneInvalidSequenceError<UTF16BE>(CodeUnitsConsumed(1u));
      }

      if (IsSingleUTF16(leadCodeUnit))
      {
        return DecodeOneSuccess<UTF16BE>(leadCodeUnit, CodeUnitsConsumed(1u));
      }

      if (++current == end)
      {
        return DecodeOneIncompleteSequenceError<UTF16BE>(CodeUnitsConsumed(1u));
      }

      auto trailCodeUnit = Endian::ToSystemEndian<Endian::Big>(static_cast<char16>(*current));
      if (!IsTrailSurrogate(trailCodeUnit))
      {
        return DecodeOneInvalidSequenceError<UTF16BE>(CodeUnitsConsumed(2u));
      }

      return DecodeOneSuccess<UTF16BE>(UTF16CombineSurrogates(leadCodeUnit, trailCodeUnit),
                                       CodeUnitsConsumed(2u));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<UTF16BE> EncodeOne(char32 codePoint) noexcept
    {
      if (IsNonUnicodeCharacter(codePoint))
      {
        return EncodeOneUnmappableCodePointError<UTF16BE>();
      }

      if (codePoint < 0x10000)
      {
        return EncodeOneSuccess<UTF16BE>(Endian::ToBigEndian(static_cast<char16>(codePoint)));
      }

      auto [lead, trail] = UTF16SplitSurrogates(codePoint);
      return EncodeOneSuccess<UTF16BE>(Endian::ToBigEndian(lead), Endian::ToBigEndian(trail));
    }
  };

  static_assert(Encoding<UTF16BE>);

#pragma endregion

#pragma region UTF32

  /// @brief The UTF-32LE Encoding.
  struct UTF32LE : public DefaultEncodingTraits
  {
    using code_unit = char32;

    constexpr static EncodingId Id = EncodingId::UTF32LE;
    constexpr static utf8_stringview Name = u8"UTF-32LE";
    constexpr static Array<utf8_stringview, 2uz> Aliases = {u8"utf-32le", u8"csutf32le"};

    constexpr static size_t MaxEncodeCodeUnits = 1uz;
    constexpr static bool IsEncodeLossless = true;
    constexpr static Array<code_unit, 1uz> ReplacementCodeUnits = {0xFFFD};
    constexpr static Array<code_unit, 4uz> BOM = {0xFF, 0xFE, 0x00, 0x00};

    template <CodeUnitInputIterator<UTF32LE> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<UTF32LE> DecodeOne(TIterator current,
                                                                       TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      char32 codeUnit = static_cast<char32>(*current);
      char32 codePoint = Endian::ToSystemEndian<Endian::Little>(codeUnit);
      if (IsNonUnicodeCharacter(codePoint))
      {
        return DecodeOneInvalidSequenceError<UTF32LE>();
      }

      return DecodeOneSuccess<UTF32LE>(codePoint);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<UTF32LE> EncodeOne(char32 codePoint) noexcept
    {
      if (IsNonUnicodeCharacter(codePoint))
      {
        return EncodeOneUnmappableCodePointError<UTF32LE>();
      }

      return EncodeOneSuccess<UTF32LE>(Endian::ToLittleEndian(codePoint));
    }
  };

  static_assert(Encoding<UTF32LE>);

  /// @brief The UTF-32BE Encoding.
  struct UTF32BE : public DefaultEncodingTraits
  {
    using code_unit = char32;

    constexpr static EncodingId Id = EncodingId::UTF32BE;
    constexpr static utf8_stringview Name = u8"UTF-32BE";
    constexpr static Array<utf8_stringview, 2uz> Aliases = {u8"utf-32be", u8"csutf32be"};

    constexpr static size_t MaxEncodeCodeUnits = 1uz;
    constexpr static bool IsEncodeLossless = true;
    constexpr static Array<code_unit, 1uz> ReplacementCodeUnits = {0xFFFD};
    constexpr static Array<code_unit, 4uz> BOM = {0x00, 0x00, 0xFE, 0xFF};

    template <CodeUnitInputIterator<UTF32BE> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<UTF32BE> DecodeOne(TIterator current,
                                                                       TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      char32 codeUnit = static_cast<char32>(*current);
      char32 codePoint = Endian::ToSystemEndian<Endian::Big>(codeUnit);
      if (IsNonUnicodeCharacter(codePoint))
      {
        return DecodeOneInvalidSequenceError<UTF32BE>();
      }

      return DecodeOneSuccess<UTF32BE>(codePoint);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<UTF32BE> EncodeOne(char32 codePoint) noexcept
    {
      if (IsNonUnicodeCharacter(codePoint))
      {
        return EncodeOneUnmappableCodePointError<UTF32BE>();
      }

      return EncodeOneSuccess<UTF32BE>(Endian::ToBigEndian(codePoint));
    }
  };

  static_assert(Encoding<UTF32BE>);

#pragma endregion
}