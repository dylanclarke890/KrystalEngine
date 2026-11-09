#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"

namespace krys::text
{
#pragma region ISO_8859_1

  /// @brief The ISO/IEC 8859-1 (Latin1/Western European) encoding.
  /// @note-1 Matches Unicode's encoding of the first 256 bytes one-to-one.
  /// @note-2 This encoding is usually treated as Windows-1252.
  struct ISO_8859_1 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_1;
    constexpr static utf8_stringview Name = u8"ISO-8859-1";
    constexpr static Array<utf8_stringview, 2uz> Aliases = {u8"iso-8859-1", u8"latin-1"};

    template <CodeUnitInputIterator<ISO_8859_1> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_1> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      char32 codeUnit = CodeUnitValue(*current);
      if (codeUnit > MaxExtendedASCIIChar)
      {
        return DecodeOneInvalidSequenceError<ISO_8859_1>();
      }

      return DecodeOneSuccess<ISO_8859_1>(codeUnit);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_1> EncodeOne(char32 codePoint) noexcept
    {
      if (codePoint > MaxExtendedASCIIChar)
      {
        return EncodeOneUnmappableCodePointError<ISO_8859_1>();
      }

      return EncodeOneSuccess<ISO_8859_1>(static_cast<code_unit>(codePoint));
    }
  };

  static_assert(Encoding<ISO_8859_1>);

#pragma endregion

#pragma region ISO_8859_1_1985

  /// @brief The encoding that matches the ISO/IEC 8859-1 specification published in 1985.
  struct ISO_8859_1_1985 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_1_1985;
    constexpr static utf8_stringview Name = u8"ISO-8859-1-1985";
    constexpr static Array<utf8_stringview, 1uz> Aliases = {u8"iso-8859-1-1985"};

    template <CodeUnitInputIterator<ISO_8859_1_1985> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_1_1985> DecodeOne(TIterator current,
                                                                               TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      char32 codeUnit = CodeUnitValue(*current);
      if (!(codeUnit >= 0x20 && codeUnit <= 0x7E) && !(codeUnit >= 0xA0 && codeUnit <= 0xFF))
      {
        return DecodeOneInvalidSequenceError<ISO_8859_1_1985>();
      }

      // Special cases for 1985
      if (codeUnit == 0xD7)
      {
        return DecodeOneSuccess<ISO_8859_1_1985>(0x0152);
      }
      if (codeUnit == 0xF7)
      {
        return DecodeOneSuccess<ISO_8859_1_1985>(0x0153);
      }

      return DecodeOneSuccess<ISO_8859_1_1985>(codeUnit);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_1_1985> EncodeOne(char32 codePoint) noexcept
    {
      if (!(codePoint >= 0x20 && codePoint <= 0x7E) && !(codePoint >= 0xA0 && codePoint <= 0xFF))
      {
        return EncodeOneUnmappableCodePointError<ISO_8859_1_1985>();
      }

      // Special cases for 1985
      if (codePoint == 0x0152)
      {
        return EncodeOneSuccess<ISO_8859_1_1985>(static_cast<code_unit>(0xD7));
      }
      if (codePoint == 0x0153)
      {
        return EncodeOneSuccess<ISO_8859_1_1985>(static_cast<code_unit>(0xF7));
      }

      return EncodeOneSuccess<ISO_8859_1_1985>(static_cast<code_unit>(codePoint));
    }
  };

  static_assert(Encoding<ISO_8859_1_1985>);

#pragma endregion

#pragma region ISO_8859_1_1998

  /// @brief The encoding that matches ISO/IEC 8859-1 specification published in 1998.
  struct ISO_8859_1_1998 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_1_1998;
    constexpr static utf8_stringview Name = u8"ISO-8859-1-1998";
    constexpr static Array<utf8_stringview, 1uz> Aliases = {u8"iso-8859-1-1998"};

    template <CodeUnitInputIterator<ISO_8859_1_1998> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_1_1998> DecodeOne(TIterator current,
                                                                               TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      char32 codeUnit = CodeUnitValue(*current);
      if (!(codeUnit >= 0x20 && codeUnit <= 0x7E) && !(codeUnit >= 0xA0 && codeUnit <= 0xFF))
      {
        return DecodeOneInvalidSequenceError<ISO_8859_1_1998>();
      }

      return DecodeOneSuccess<ISO_8859_1_1998>(codeUnit);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_1_1998> EncodeOne(char32 codePoint) noexcept
    {
      if (!(codePoint >= 0x20 && codePoint <= 0x7E) && !(codePoint >= 0xA0 && codePoint <= 0xFF))
      {
        return EncodeOneUnmappableCodePointError<ISO_8859_1_1998>();
      }

      return EncodeOneSuccess<ISO_8859_1_1998>(static_cast<code_unit>(codePoint));
    }
  };

  static_assert(Encoding<ISO_8859_1_1998>);

#pragma endregion

#pragma region ISO_8859_2

  /// @brief The ISO/IEC 8859-2 (Latin2/Central European) encoding.
  struct ISO_8859_2 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_2;
    constexpr static utf8_stringview Name = u8"ISO-8859-2";
    constexpr static Array<utf8_stringview, 9uz> Aliases = {
      u8"csisolatin2", u8"iso-8859-2",      u8"iso-ir-101", u8"iso8859-2", u8"iso88592",
      u8"iso_8859-2",  u8"iso_8859-2:1987", u8"l2",         u8"latin2"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C,
       0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099,
       0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A,
       0x00A7, 0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B, 0x00B0, 0x0105, 0x02DB, 0x0142,
       0x00B4, 0x013E, 0x015B, 0x02C7, 0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C, 0x0154,
       0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD,
       0x00CE, 0x010E, 0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA,
       0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF, 0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
       0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F, 0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4,
       0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9}};

    template <CodeUnitInputIterator<ISO_8859_2> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_2> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_2>(CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_2> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_2>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_2>);

#pragma endregion

#pragma region ISO_8859_3

  /// @brief The ISO/IEC 8859-3 (Latin3/South European) encoding.
  struct ISO_8859_3 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_3;
    constexpr static utf8_stringview Name = u8"ISO-8859-3";
    constexpr static Array<utf8_stringview, 9uz> Aliases = {
      u8"iso-8859-3", u8"csisolatin3",     u8"iso-ir-109", u8"iso8859-3", u8"iso88593",
      u8"iso_8859-3", u8"iso_8859-3:1988", u8"l3",         u8"latin3"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080,       0x0081,       0x0082, 0x0083, 0x0084,       0x0085, 0x0086, 0x0087, 0x0088,
       0x0089,       0x008A,       0x008B, 0x008C, 0x008D,       0x008E, 0x008F, 0x0090, 0x0091,
       0x0092,       0x0093,       0x0094, 0x0095, 0x0096,       0x0097, 0x0098, 0x0099, 0x009A,
       0x009B,       0x009C,       0x009D, 0x009E, 0x009F,       0x00A0, 0x0126, 0x02D8, 0x00A3,
       0x00A4,       UnmappedByte, 0x0124, 0x00A7, 0x00A8,       0x0130, 0x015E, 0x011E, 0x0134,
       0x00AD,       UnmappedByte, 0x017B, 0x00B0, 0x0127,       0x00B2, 0x00B3, 0x00B4, 0x00B5,
       0x0125,       0x00B7,       0x00B8, 0x0131, 0x015F,       0x011F, 0x0135, 0x00BD, UnmappedByte,
       0x017C,       0x00C0,       0x00C1, 0x00C2, UnmappedByte, 0x00C4, 0x010A, 0x0108, 0x00C7,
       0x00C8,       0x00C9,       0x00CA, 0x00CB, 0x00CC,       0x00CD, 0x00CE, 0x00CF, UnmappedByte,
       0x00D1,       0x00D2,       0x00D3, 0x00D4, 0x0120,       0x00D6, 0x00D7, 0x011C, 0x00D9,
       0x00DA,       0x00DB,       0x00DC, 0x016C, 0x015C,       0x00DF, 0x00E0, 0x00E1, 0x00E2,
       UnmappedByte, 0x00E4,       0x010B, 0x0109, 0x00E7,       0x00E8, 0x00E9, 0x00EA, 0x00EB,
       0x00EC,       0x00ED,       0x00EE, 0x00EF, UnmappedByte, 0x00F1, 0x00F2, 0x00F3, 0x00F4,
       0x0121,       0x00F6,       0x00F7, 0x011D, 0x00F9,       0x00FA, 0x00FB, 0x00FC, 0x016D,
       0x015D,       0x02D9}};

    template <CodeUnitInputIterator<ISO_8859_3> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_3> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_3, MightBeInvalidEntry(true)>(
        CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_3> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_3>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_3>);

#pragma endregion

#pragma region ISO_8859_4

  /// @brief The ISO/IEC 8859-4 (Latin4/North European) encoding.
  struct ISO_8859_4 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_4;
    constexpr static utf8_stringview Name = u8"ISO-8859-4";
    constexpr static Array<utf8_stringview, 9uz> Aliases = {
      u8"iso-8859-4", u8"csisolatin4",     u8"iso-ir-110", u8"iso8859-4", u8"iso88594",
      u8"iso_8859-4", u8"iso_8859-4:1988", u8"l4",         u8"latin4"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C,
       0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099,
       0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x0104, 0x0138, 0x0156, 0x00A4, 0x0128, 0x013B,
       0x00A7, 0x00A8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00AD, 0x017D, 0x00AF, 0x00B0, 0x0105, 0x02DB, 0x0157,
       0x00B4, 0x0129, 0x013C, 0x02C7, 0x00B8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014A, 0x017E, 0x014B, 0x0100,
       0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD,
       0x00CE, 0x012A, 0x0110, 0x0145, 0x014C, 0x0136, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x0172, 0x00DA,
       0x00DB, 0x00DC, 0x0168, 0x016A, 0x00DF, 0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
       0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x012B, 0x0111, 0x0146, 0x014D, 0x0137, 0x00F4,
       0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x0169, 0x016B, 0x02D9}};

    template <CodeUnitInputIterator<ISO_8859_4> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_4> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_4>(CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_4> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_4>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_4>);

#pragma endregion

#pragma region ISO_8859_5

  /// @brief The ISO/IEC 8859-5 (Latin/Cyrillic) encoding.
  struct ISO_8859_5 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_5;
    constexpr static utf8_stringview Name = u8"ISO-8859-5";
    constexpr static Array<utf8_stringview, 8uz> Aliases = {
      u8"iso-8859-5", u8"csisolatincyrillic", u8"cyrillic",   u8"iso-ir-144",
      u8"iso8859-5",  u8"iso88595",           u8"iso_8859-5", u8"iso_8859-5:1988"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C,
       0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099,
       0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406,
       0x0407, 0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x00AD, 0x040E, 0x040F, 0x0410, 0x0411, 0x0412, 0x0413,
       0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, 0x0420,
       0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D,
       0x042E, 0x042F, 0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A,
       0x043B, 0x043C, 0x043D, 0x043E, 0x043F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
       0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F, 0x2116, 0x0451, 0x0452, 0x0453, 0x0454,
       0x0455, 0x0456, 0x0457, 0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F}};

    template <CodeUnitInputIterator<ISO_8859_5> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_5> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_5>(CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_5> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_5>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_5>);

#pragma endregion

#pragma region ISO_8859_6

  /// @brief The ISO/IEC 8859-6 (Latin/Arabic) encoding.
  struct ISO_8859_6 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_6;
    constexpr static utf8_stringview Name = u8"ISO-8859-6";
    constexpr static Array<utf8_stringview, 14uz> Aliases = {
      u8"iso-8859-6",       u8"arabic",   u8"asmo-708",     u8"csiso88596e",    u8"csiso88596i",
      u8"csisolatinarabic", u8"ecma-114", u8"iso-8859-6-e", u8"iso-8859-6-i",   u8"iso-ir-127",
      u8"iso8859-6",        u8"iso88596", u8"iso_8859-6",   u8"iso_8859-6:1987"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080,       0x0081,       0x0082,       0x0083,       0x0084,       0x0085,       0x0086,
       0x0087,       0x0088,       0x0089,       0x008A,       0x008B,       0x008C,       0x008D,
       0x008E,       0x008F,       0x0090,       0x0091,       0x0092,       0x0093,       0x0094,
       0x0095,       0x0096,       0x0097,       0x0098,       0x0099,       0x009A,       0x009B,
       0x009C,       0x009D,       0x009E,       0x009F,       0x00A0,       UnmappedByte, UnmappedByte,
       UnmappedByte, 0x00A4,       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte, 0x060C,       0x00AD,       UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte, UnmappedByte, 0x061B,       UnmappedByte, UnmappedByte, UnmappedByte,
       0x061F,       UnmappedByte, 0x0621,       0x0622,       0x0623,       0x0624,       0x0625,
       0x0626,       0x0627,       0x0628,       0x0629,       0x062A,       0x062B,       0x062C,
       0x062D,       0x062E,       0x062F,       0x0630,       0x0631,       0x0632,       0x0633,
       0x0634,       0x0635,       0x0636,       0x0637,       0x0638,       0x0639,       0x063A,
       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, 0x0640,       0x0641,
       0x0642,       0x0643,       0x0644,       0x0645,       0x0646,       0x0647,       0x0648,
       0x0649,       0x064A,       0x064B,       0x064C,       0x064D,       0x064E,       0x064F,
       0x0650,       0x0651,       0x0652,       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte}};

    template <CodeUnitInputIterator<ISO_8859_6> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_6> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_6, MightBeInvalidEntry(true)>(
        CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_6> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_6>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_6>);

#pragma endregion

#pragma region ISO_8859_7

  /// @brief The ISO/IEC 8859-7 (Latin/Greek) encoding.
  struct ISO_8859_7 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_7;
    constexpr static utf8_stringview Name = u8"ISO-8859-7";
    constexpr static Array<utf8_stringview, 12uz> Aliases = {
      u8"iso-8859-7", u8"csisolatingreek", u8"ecma-118",        u8"elot_928",
      u8"greek",      u8"greek8",          u8"iso-ir-126",      u8"iso8859-7",
      u8"iso88597",   u8"iso_8859-7",      u8"iso_8859-7:1987", u8"sun_eu_greek"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082,       0x0083, 0x0084, 0x0085, 0x0086,       0x0087,      0x0088, 0x0089,
       0x008A, 0x008B, 0x008C,       0x008D, 0x008E, 0x008F, 0x0090,       0x0091,      0x0092, 0x0093,
       0x0094, 0x0095, 0x0096,       0x0097, 0x0098, 0x0099, 0x009A,       0x009B,      0x009C, 0x009D,
       0x009E, 0x009F, 0x00A0,       0x2018, 0x2019, 0x00A3, 0x20AC,       0x20AF,      0x00A6, 0x00A7,
       0x00A8, 0x00A9, 0x037A,       0x00AB, 0x00AC, 0x00AD, UnmappedByte, 0x2015,      0x00B0, 0x00B1,
       0x00B2, 0x00B3, 0x0384,       0x0385, 0x0386, 0x00B7, 0x0388,       0x0389,      0x038A, 0x00BB,
       0x038C, 0x00BD, 0x038E,       0x038F, 0x0390, 0x0391, 0x0392,       0x0393,      0x0394, 0x0395,
       0x0396, 0x0397, 0x0398,       0x0399, 0x039A, 0x039B, 0x039C,       0x039D,      0x039E, 0x039F,
       0x03A0, 0x03A1, UnmappedByte, 0x03A3, 0x03A4, 0x03A5, 0x03A6,       0x03A7,      0x03A8, 0x03A9,
       0x03AA, 0x03AB, 0x03AC,       0x03AD, 0x03AE, 0x03AF, 0x03B0,       0x03B1,      0x03B2, 0x03B3,
       0x03B4, 0x03B5, 0x03B6,       0x03B7, 0x03B8, 0x03B9, 0x03BA,       0x03BB,      0x03BC, 0x03BD,
       0x03BE, 0x03BF, 0x03C0,       0x03C1, 0x03C2, 0x03C3, 0x03C4,       0x03C5,      0x03C6, 0x03C7,
       0x03C8, 0x03C9, 0x03CA,       0x03CB, 0x03CC, 0x03CD, 0x03CE,       UnmappedByte}};

    template <CodeUnitInputIterator<ISO_8859_7> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_7> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_7, MightBeInvalidEntry(true)>(
        CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_7> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_7>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_7>);

#pragma endregion

#pragma region ISO_8859_8

  /// @brief The ISO/IEC 8859-8 (Latin/Hebrew) encoding.
  struct ISO_8859_8 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_8;
    constexpr static utf8_stringview Name = u8"ISO-8859-8";
    constexpr static Array<utf8_stringview, 11uz> Aliases = {
      u8"iso-8859-8", u8"csiso88598e", u8"csisolatinhebrew", u8"hebrew",     u8"iso-8859-8-e",
      u8"iso-ir-138", u8"iso8859-8",   u8"iso88598",         u8"iso_8859-8", u8"iso_8859-8:1988",
      u8"visual"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080,       0x0081,       0x0082,       0x0083,       0x0084,       0x0085,       0x0086,
       0x0087,       0x0088,       0x0089,       0x008A,       0x008B,       0x008C,       0x008D,
       0x008E,       0x008F,       0x0090,       0x0091,       0x0092,       0x0093,       0x0094,
       0x0095,       0x0096,       0x0097,       0x0098,       0x0099,       0x009A,       0x009B,
       0x009C,       0x009D,       0x009E,       0x009F,       0x00A0,       UnmappedByte, 0x00A2,
       0x00A3,       0x00A4,       0x00A5,       0x00A6,       0x00A7,       0x00A8,       0x00A9,
       0x00D7,       0x00AB,       0x00AC,       0x00AD,       0x00AE,       0x00AF,       0x00B0,
       0x00B1,       0x00B2,       0x00B3,       0x00B4,       0x00B5,       0x00B6,       0x00B7,
       0x00B8,       0x00B9,       0x00F7,       0x00BB,       0x00BC,       0x00BD,       0x00BE,
       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte,
       UnmappedByte, UnmappedByte, UnmappedByte, UnmappedByte, 0x2017,       0x05D0,       0x05D1,
       0x05D2,       0x05D3,       0x05D4,       0x05D5,       0x05D6,       0x05D7,       0x05D8,
       0x05D9,       0x05DA,       0x05DB,       0x05DC,       0x05DD,       0x05DE,       0x05DF,
       0x05E0,       0x05E1,       0x05E2,       0x05E3,       0x05E4,       0x05E5,       0x05E6,
       0x05E7,       0x05E8,       0x05E9,       0x05EA,       UnmappedByte, UnmappedByte, 0x200E,
       0x200F,       UnmappedByte}};

    template <CodeUnitInputIterator<ISO_8859_8> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_8> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_8, MightBeInvalidEntry(true)>(
        CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_8> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_8>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_8>);

#pragma endregion

#pragma region ISO_8859_10

  /// @brief The encoding that matches the ISO/IEC 8859-10 encoding specification.
  struct ISO_8859_10 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_10;
    constexpr static utf8_stringview Name = u8"ISO-8859-10";
    constexpr static Array<utf8_stringview, 7uz> Aliases = {
      u8"iso-8859-10", u8"csisolatin6", u8"iso-ir-157", u8"iso8859-10", u8"iso885910", u8"l6", u8"latin6"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C,
       0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099,
       0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x0104, 0x0112, 0x0122, 0x012A, 0x0128, 0x0136,
       0x00A7, 0x013B, 0x0110, 0x0160, 0x0166, 0x017D, 0x00AD, 0x016A, 0x014A, 0x00B0, 0x0105, 0x0113, 0x0123,
       0x012B, 0x0129, 0x0137, 0x00B7, 0x013C, 0x0111, 0x0161, 0x0167, 0x017E, 0x2015, 0x016B, 0x014B, 0x0100,
       0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD,
       0x00CE, 0x00CF, 0x00D0, 0x0145, 0x014C, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0168, 0x00D8, 0x0172, 0x00DA,
       0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
       0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x0146, 0x014D, 0x00F3, 0x00F4,
       0x00F5, 0x00F6, 0x0169, 0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x0138}};

    template <CodeUnitInputIterator<ISO_8859_10> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_10> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_10>(CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_10> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_10>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_10>);

#pragma endregion

#pragma region ISO_8859_13

  /// @brief The encoding that matches the ISO/IEC 8859-13 encoding specification.
  struct ISO_8859_13 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_13;
    constexpr static utf8_stringview Name = u8"ISO-8859-13";
    constexpr static Array<utf8_stringview, 3uz> Aliases = {u8"iso-8859-13", u8"iso8859-13", u8"iso885913"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C,
       0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099,
       0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x201D, 0x00A2, 0x00A3, 0x00A4, 0x201E, 0x00A6,
       0x00A7, 0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6, 0x00B0, 0x00B1, 0x00B2, 0x00B3,
       0x201C, 0x00B5, 0x00B6, 0x00B7, 0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6, 0x0104,
       0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112, 0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136,
       0x012A, 0x013B, 0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7, 0x0172, 0x0141, 0x015A,
       0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF, 0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
       0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C, 0x0161, 0x0144, 0x0146, 0x00F3, 0x014D,
       0x00F5, 0x00F6, 0x00F7, 0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x2019}};

    template <CodeUnitInputIterator<ISO_8859_13> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_13> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_13>(CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_13> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_13>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_13>);

#pragma endregion

#pragma region ISO_8859_14

  /// @brief The encoding that matches the ISO/IEC 8859-14 encoding specification.
  struct ISO_8859_14 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_14;
    constexpr static utf8_stringview Name = u8"ISO-8859-14";
    constexpr static Array<utf8_stringview, 3uz> Aliases = {u8"iso-8859-14", u8"iso8859-14", u8"iso885914"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C,
       0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099,
       0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x1E02, 0x1E03, 0x00A3, 0x010A, 0x010B, 0x1E0A,
       0x00A7, 0x1E80, 0x00A9, 0x1E82, 0x1E0B, 0x1EF2, 0x00AD, 0x00AE, 0x0178, 0x1E1E, 0x1E1F, 0x0120, 0x0121,
       0x1E40, 0x1E41, 0x00B6, 0x1E56, 0x1E81, 0x1E57, 0x1E83, 0x1E60, 0x1EF3, 0x1E84, 0x1E85, 0x1E61, 0x00C0,
       0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD,
       0x00CE, 0x00CF, 0x0174, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x1E6A, 0x00D8, 0x00D9, 0x00DA,
       0x00DB, 0x00DC, 0x00DD, 0x0176, 0x00DF, 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
       0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x0175, 0x00F1, 0x00F2, 0x00F3, 0x00F4,
       0x00F5, 0x00F6, 0x1E6B, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x0177, 0x00FF}};

    template <CodeUnitInputIterator<ISO_8859_14> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_14> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_14>(CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_14> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_14>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_14>);

#pragma endregion

#pragma region ISO_8859_15

  /// @brief The encoding that matches the ISO/IEC 8859-15 encoding specification.
  struct ISO_8859_15 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_15;
    constexpr static utf8_stringview Name = u8"ISO-8859-15";
    constexpr static Array<utf8_stringview, 6uz> Aliases = {u8"iso-8859-15", u8"csisolatin9", u8"iso8859-15",
                                                            u8"iso885915",   u8"iso_8859-15", u8"l9"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C,
       0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099,
       0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AC, 0x00A5, 0x0160,
       0x00A7, 0x0161, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00B1, 0x00B2, 0x00B3,
       0x017D, 0x00B5, 0x00B6, 0x00B7, 0x017E, 0x00B9, 0x00BA, 0x00BB, 0x0152, 0x0153, 0x0178, 0x00BF, 0x00C0,
       0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD,
       0x00CE, 0x00CF, 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA,
       0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
       0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4,
       0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF}};

    template <CodeUnitInputIterator<ISO_8859_15> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_15> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_15>(CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_15> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_15>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_15>);

#pragma endregion

#pragma region ISO_8859_16

  /// @brief The encoding that matches the ISO/IEC 8859-16 encoding specification.
  struct ISO_8859_16 : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ISO_8859_16;
    constexpr static utf8_stringview Name = u8"ISO-8859-16";
    constexpr static Array<utf8_stringview, 1uz> Aliases = {u8"iso-8859-16"};

    constexpr static Array<char32, 128uz> LookupMap = {
      {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C,
       0x008D, 0x008E, 0x008F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099,
       0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x0104, 0x0105, 0x0141, 0x20AC, 0x201E, 0x0160,
       0x00A7, 0x0161, 0x00A9, 0x0218, 0x00AB, 0x0179, 0x00AD, 0x017A, 0x017B, 0x00B0, 0x00B1, 0x010C, 0x0142,
       0x017D, 0x201D, 0x00B6, 0x00B7, 0x017E, 0x010D, 0x0219, 0x00BB, 0x0152, 0x0153, 0x0178, 0x017C, 0x00C0,
       0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0106, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD,
       0x00CE, 0x00CF, 0x0110, 0x0143, 0x00D2, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x015A, 0x0170, 0x00D9, 0x00DA,
       0x00DB, 0x00DC, 0x0118, 0x021A, 0x00DF, 0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x0107, 0x00E6, 0x00E7,
       0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x0111, 0x0144, 0x00F2, 0x00F3, 0x00F4,
       0x0151, 0x00F6, 0x015B, 0x0171, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0119, 0x021B, 0x00FF}};

    template <CodeUnitInputIterator<ISO_8859_16> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_16> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_16>(CodeUnitValue(*current));
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_16> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_16>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_16>);

#pragma endregion
}