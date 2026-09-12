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

      char32 codeUnit = static_cast<char32>(*current);
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

      char32 codeUnit = static_cast<char32>(*current);
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

      char32 codeUnit = static_cast<char32>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
      {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
      {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
      {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
      {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x0104},  {34, 0x02D8},
      {35, 0x0141},  {36, 0x00A4},  {37, 0x013D},  {38, 0x015A},  {39, 0x00A7},  {40, 0x00A8},  {41, 0x0160},
      {42, 0x015E},  {43, 0x0164},  {44, 0x0179},  {45, 0x00AD},  {46, 0x017D},  {47, 0x017B},  {48, 0x00B0},
      {49, 0x0105},  {50, 0x02DB},  {51, 0x0142},  {52, 0x00B4},  {53, 0x013E},  {54, 0x015B},  {55, 0x02C7},
      {56, 0x00B8},  {57, 0x0161},  {58, 0x015F},  {59, 0x0165},  {60, 0x017A},  {61, 0x02DD},  {62, 0x017E},
      {63, 0x017C},  {64, 0x0154},  {65, 0x00C1},  {66, 0x00C2},  {67, 0x0102},  {68, 0x00C4},  {69, 0x0139},
      {70, 0x0106},  {71, 0x00C7},  {72, 0x010C},  {73, 0x00C9},  {74, 0x0118},  {75, 0x00CB},  {76, 0x011A},
      {77, 0x00CD},  {78, 0x00CE},  {79, 0x010E},  {80, 0x0110},  {81, 0x0143},  {82, 0x0147},  {83, 0x00D3},
      {84, 0x00D4},  {85, 0x0150},  {86, 0x00D6},  {87, 0x00D7},  {88, 0x0158},  {89, 0x016E},  {90, 0x00DA},
      {91, 0x0170},  {92, 0x00DC},  {93, 0x00DD},  {94, 0x0162},  {95, 0x00DF},  {96, 0x0155},  {97, 0x00E1},
      {98, 0x00E2},  {99, 0x0103},  {100, 0x00E4}, {101, 0x013A}, {102, 0x0107}, {103, 0x00E7}, {104, 0x010D},
      {105, 0x00E9}, {106, 0x0119}, {107, 0x00EB}, {108, 0x011B}, {109, 0x00ED}, {110, 0x00EE}, {111, 0x010F},
      {112, 0x0111}, {113, 0x0144}, {114, 0x0148}, {115, 0x00F3}, {116, 0x00F4}, {117, 0x0151}, {118, 0x00F6},
      {119, 0x00F7}, {120, 0x0159}, {121, 0x016F}, {122, 0x00FA}, {123, 0x0171}, {124, 0x00FC}, {125, 0x00FD},
      {126, 0x0163}, {127, 0x02D9},
    }};

    template <CodeUnitInputIterator<ISO_8859_2> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_2> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_2>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},      {2, 0x0082},      {3, 0x0083},      {4, 0x0084},       {5, 0x0085},
      {6, 0x0086},   {7, 0x0087},      {8, 0x0088},      {9, 0x0089},      {10, 0x008A},      {11, 0x008B},
      {12, 0x008C},  {13, 0x008D},     {14, 0x008E},     {15, 0x008F},     {16, 0x0090},      {17, 0x0091},
      {18, 0x0092},  {19, 0x0093},     {20, 0x0094},     {21, 0x0095},     {22, 0x0096},      {23, 0x0097},
      {24, 0x0098},  {25, 0x0099},     {26, 0x009A},     {27, 0x009B},     {28, 0x009C},      {29, 0x009D},
      {30, 0x009E},  {31, 0x009F},     {32, 0x00A0},     {33, 0x0126},     {34, 0x02D8},      {35, 0x00A3},
      {36, 0x00A4},  InvalidEntry(37), {38, 0x0124},     {39, 0x00A7},     {40, 0x00A8},      {41, 0x0130},
      {42, 0x015E},  {43, 0x011E},     {44, 0x0134},     {45, 0x00AD},     InvalidEntry(46),  {47, 0x017B},
      {48, 0x00B0},  {49, 0x0127},     {50, 0x00B2},     {51, 0x00B3},     {52, 0x00B4},      {53, 0x00B5},
      {54, 0x0125},  {55, 0x00B7},     {56, 0x00B8},     {57, 0x0131},     {58, 0x015F},      {59, 0x011F},
      {60, 0x0135},  {61, 0x00BD},     InvalidEntry(62), {63, 0x017C},     {64, 0x00C0},      {65, 0x00C1},
      {66, 0x00C2},  InvalidEntry(67), {68, 0x00C4},     {69, 0x010A},     {70, 0x0108},      {71, 0x00C7},
      {72, 0x00C8},  {73, 0x00C9},     {74, 0x00CA},     {75, 0x00CB},     {76, 0x00CC},      {77, 0x00CD},
      {78, 0x00CE},  {79, 0x00CF},     InvalidEntry(80), {81, 0x00D1},     {82, 0x00D2},      {83, 0x00D3},
      {84, 0x00D4},  {85, 0x0120},     {86, 0x00D6},     {87, 0x00D7},     {88, 0x011C},      {89, 0x00D9},
      {90, 0x00DA},  {91, 0x00DB},     {92, 0x00DC},     {93, 0x016C},     {94, 0x015C},      {95, 0x00DF},
      {96, 0x00E0},  {97, 0x00E1},     {98, 0x00E2},     InvalidEntry(99), {100, 0x00E4},     {101, 0x010B},
      {102, 0x0109}, {103, 0x00E7},    {104, 0x00E8},    {105, 0x00E9},    {106, 0x00EA},     {107, 0x00EB},
      {108, 0x00EC}, {109, 0x00ED},    {110, 0x00EE},    {111, 0x00EF},    InvalidEntry(112), {113, 0x00F1},
      {114, 0x00F2}, {115, 0x00F3},    {116, 0x00F4},    {117, 0x0121},    {118, 0x00F6},     {119, 0x00F7},
      {120, 0x011D}, {121, 0x00F9},    {122, 0x00FA},    {123, 0x00FB},    {124, 0x00FC},     {125, 0x016D},
      {126, 0x015D}, {127, 0x02D9},
    }};

    template <CodeUnitInputIterator<ISO_8859_3> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_3> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_3, MightBeInvalidEntry(true)>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
      {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
      {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
      {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
      {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x0104},  {34, 0x0138},
      {35, 0x0156},  {36, 0x00A4},  {37, 0x0128},  {38, 0x013B},  {39, 0x00A7},  {40, 0x00A8},  {41, 0x0160},
      {42, 0x0112},  {43, 0x0122},  {44, 0x0166},  {45, 0x00AD},  {46, 0x017D},  {47, 0x00AF},  {48, 0x00B0},
      {49, 0x0105},  {50, 0x02DB},  {51, 0x0157},  {52, 0x00B4},  {53, 0x0129},  {54, 0x013C},  {55, 0x02C7},
      {56, 0x00B8},  {57, 0x0161},  {58, 0x0113},  {59, 0x0123},  {60, 0x0167},  {61, 0x014A},  {62, 0x017E},
      {63, 0x014B},  {64, 0x0100},  {65, 0x00C1},  {66, 0x00C2},  {67, 0x00C3},  {68, 0x00C4},  {69, 0x00C5},
      {70, 0x00C6},  {71, 0x012E},  {72, 0x010C},  {73, 0x00C9},  {74, 0x0118},  {75, 0x00CB},  {76, 0x0116},
      {77, 0x00CD},  {78, 0x00CE},  {79, 0x012A},  {80, 0x0110},  {81, 0x0145},  {82, 0x014C},  {83, 0x0136},
      {84, 0x00D4},  {85, 0x00D5},  {86, 0x00D6},  {87, 0x00D7},  {88, 0x00D8},  {89, 0x0172},  {90, 0x00DA},
      {91, 0x00DB},  {92, 0x00DC},  {93, 0x0168},  {94, 0x016A},  {95, 0x00DF},  {96, 0x0101},  {97, 0x00E1},
      {98, 0x00E2},  {99, 0x00E3},  {100, 0x00E4}, {101, 0x00E5}, {102, 0x00E6}, {103, 0x012F}, {104, 0x010D},
      {105, 0x00E9}, {106, 0x0119}, {107, 0x00EB}, {108, 0x0117}, {109, 0x00ED}, {110, 0x00EE}, {111, 0x012B},
      {112, 0x0111}, {113, 0x0146}, {114, 0x014D}, {115, 0x0137}, {116, 0x00F4}, {117, 0x00F5}, {118, 0x00F6},
      {119, 0x00F7}, {120, 0x00F8}, {121, 0x0173}, {122, 0x00FA}, {123, 0x00FB}, {124, 0x00FC}, {125, 0x0169},
      {126, 0x016B}, {127, 0x02D9},
    }};

    template <CodeUnitInputIterator<ISO_8859_4> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_4> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_4>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
      {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
      {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
      {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
      {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x0401},  {34, 0x0402},
      {35, 0x0403},  {36, 0x0404},  {37, 0x0405},  {38, 0x0406},  {39, 0x0407},  {40, 0x0408},  {41, 0x0409},
      {42, 0x040A},  {43, 0x040B},  {44, 0x040C},  {45, 0x00AD},  {46, 0x040E},  {47, 0x040F},  {48, 0x0410},
      {49, 0x0411},  {50, 0x0412},  {51, 0x0413},  {52, 0x0414},  {53, 0x0415},  {54, 0x0416},  {55, 0x0417},
      {56, 0x0418},  {57, 0x0419},  {58, 0x041A},  {59, 0x041B},  {60, 0x041C},  {61, 0x041D},  {62, 0x041E},
      {63, 0x041F},  {64, 0x0420},  {65, 0x0421},  {66, 0x0422},  {67, 0x0423},  {68, 0x0424},  {69, 0x0425},
      {70, 0x0426},  {71, 0x0427},  {72, 0x0428},  {73, 0x0429},  {74, 0x042A},  {75, 0x042B},  {76, 0x042C},
      {77, 0x042D},  {78, 0x042E},  {79, 0x042F},  {80, 0x0430},  {81, 0x0431},  {82, 0x0432},  {83, 0x0433},
      {84, 0x0434},  {85, 0x0435},  {86, 0x0436},  {87, 0x0437},  {88, 0x0438},  {89, 0x0439},  {90, 0x043A},
      {91, 0x043B},  {92, 0x043C},  {93, 0x043D},  {94, 0x043E},  {95, 0x043F},  {96, 0x0440},  {97, 0x0441},
      {98, 0x0442},  {99, 0x0443},  {100, 0x0444}, {101, 0x0445}, {102, 0x0446}, {103, 0x0447}, {104, 0x0448},
      {105, 0x0449}, {106, 0x044A}, {107, 0x044B}, {108, 0x044C}, {109, 0x044D}, {110, 0x044E}, {111, 0x044F},
      {112, 0x2116}, {113, 0x0451}, {114, 0x0452}, {115, 0x0453}, {116, 0x0454}, {117, 0x0455}, {118, 0x0456},
      {119, 0x0457}, {120, 0x0458}, {121, 0x0459}, {122, 0x045A}, {123, 0x045B}, {124, 0x045C}, {125, 0x00A7},
      {126, 0x045E}, {127, 0x045F},
    }};

    template <CodeUnitInputIterator<ISO_8859_5> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_5> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_5>(*current);
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

    // TODO(possible-optimisation): Is padding the array to 128 entries better than using a sparse array of 83
    // entries? The latter would save memory but will be slower to search.
    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {
      {{0, 0x0080},       {1, 0x0081},       {2, 0x0082},       {3, 0x0083},       {4, 0x0084},
       {5, 0x0085},       {6, 0x0086},       {7, 0x0087},       {8, 0x0088},       {9, 0x0089},
       {10, 0x008A},      {11, 0x008B},      {12, 0x008C},      {13, 0x008D},      {14, 0x008E},
       {15, 0x008F},      {16, 0x0090},      {17, 0x0091},      {18, 0x0092},      {19, 0x0093},
       {20, 0x0094},      {21, 0x0095},      {22, 0x0096},      {23, 0x0097},      {24, 0x0098},
       {25, 0x0099},      {26, 0x009A},      {27, 0x009B},      {28, 0x009C},      {29, 0x009D},
       {30, 0x009E},      {31, 0x009F},      {32, 0x00A0},      InvalidEntry(33),  InvalidEntry(34),
       InvalidEntry(35),  {36, 0x00A4},      InvalidEntry(37),  InvalidEntry(38),  InvalidEntry(39),
       InvalidEntry(40),  InvalidEntry(41),  InvalidEntry(42),  InvalidEntry(43),  {44, 0x060C},
       {45, 0x00AD},      InvalidEntry(46),  InvalidEntry(47),  InvalidEntry(48),  InvalidEntry(49),
       InvalidEntry(50),  InvalidEntry(51),  InvalidEntry(52),  InvalidEntry(53),  InvalidEntry(54),
       InvalidEntry(55),  InvalidEntry(56),  InvalidEntry(57),  InvalidEntry(58),  {59, 0x061B},
       InvalidEntry(60),  InvalidEntry(61),  InvalidEntry(62),  {63, 0x061F},      InvalidEntry(64),
       {65, 0x0621},      {66, 0x0622},      {67, 0x0623},      {68, 0x0624},      {69, 0x0625},
       {70, 0x0626},      {71, 0x0627},      {72, 0x0628},      {73, 0x0629},      {74, 0x062A},
       {75, 0x062B},      {76, 0x062C},      {77, 0x062D},      {78, 0x062E},      {79, 0x062F},
       {80, 0x0630},      {81, 0x0631},      {82, 0x0632},      {83, 0x0633},      {84, 0x0634},
       {85, 0x0635},      {86, 0x0636},      {87, 0x0637},      {88, 0x0638},      {89, 0x0639},
       {90, 0x063A},      InvalidEntry(91),  InvalidEntry(92),  InvalidEntry(93),  InvalidEntry(94),
       InvalidEntry(95),  {96, 0x0640},      {97, 0x0641},      {98, 0x0642},      {99, 0x0643},
       {100, 0x0644},     {101, 0x0645},     {102, 0x0646},     {103, 0x0647},     {104, 0x0648},
       {105, 0x0649},     {106, 0x064A},     {107, 0x064B},     {108, 0x064C},     {109, 0x064D},
       {110, 0x064E},     {111, 0x064F},     {112, 0x0650},     {113, 0x0651},     {114, 0x0652},
       InvalidEntry(115), InvalidEntry(116), InvalidEntry(117), InvalidEntry(118), InvalidEntry(119),
       InvalidEntry(120), InvalidEntry(121), InvalidEntry(122), InvalidEntry(123), InvalidEntry(124),
       InvalidEntry(125), InvalidEntry(126), InvalidEntry(127)}};

    template <CodeUnitInputIterator<ISO_8859_6> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_6> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_6, MightBeInvalidEntry(true)>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {
      {{0, 0x0080},   {1, 0x0081},      {2, 0x0082},   {3, 0x0083},   {4, 0x0084},      {5, 0x0085},
       {6, 0x0086},   {7, 0x0087},      {8, 0x0088},   {9, 0x0089},   {10, 0x008A},     {11, 0x008B},
       {12, 0x008C},  {13, 0x008D},     {14, 0x008E},  {15, 0x008F},  {16, 0x0090},     {17, 0x0091},
       {18, 0x0092},  {19, 0x0093},     {20, 0x0094},  {21, 0x0095},  {22, 0x0096},     {23, 0x0097},
       {24, 0x0098},  {25, 0x0099},     {26, 0x009A},  {27, 0x009B},  {28, 0x009C},     {29, 0x009D},
       {30, 0x009E},  {31, 0x009F},     {32, 0x00A0},  {33, 0x2018},  {34, 0x2019},     {35, 0x00A3},
       {36, 0x20AC},  {37, 0x20AF},     {38, 0x00A6},  {39, 0x00A7},  {40, 0x00A8},     {41, 0x00A9},
       {42, 0x037A},  {43, 0x00AB},     {44, 0x00AC},  {45, 0x00AD},  InvalidEntry(46), {47, 0x2015},
       {48, 0x00B0},  {49, 0x00B1},     {50, 0x00B2},  {51, 0x00B3},  {52, 0x0384},     {53, 0x0385},
       {54, 0x0386},  {55, 0x00B7},     {56, 0x0388},  {57, 0x0389},  {58, 0x038A},     {59, 0x00BB},
       {60, 0x038C},  {61, 0x00BD},     {62, 0x038E},  {63, 0x038F},  {64, 0x0390},     {65, 0x0391},
       {66, 0x0392},  {67, 0x0393},     {68, 0x0394},  {69, 0x0395},  {70, 0x0396},     {71, 0x0397},
       {72, 0x0398},  {73, 0x0399},     {74, 0x039A},  {75, 0x039B},  {76, 0x039C},     {77, 0x039D},
       {78, 0x039E},  {79, 0x039F},     {80, 0x03A0},  {81, 0x03A1},  InvalidEntry(80), {83, 0x03A3},
       {84, 0x03A4},  {85, 0x03A5},     {86, 0x03A6},  {87, 0x03A7},  {88, 0x03A8},     {89, 0x03A9},
       {90, 0x03AA},  {91, 0x03AB},     {92, 0x03AC},  {93, 0x03AD},  {94, 0x03AE},     {95, 0x03AF},
       {96, 0x03B0},  {97, 0x03B1},     {98, 0x03B2},  {99, 0x03B3},  {100, 0x03B4},    {101, 0x03B5},
       {102, 0x03B6}, {103, 0x03B7},    {104, 0x03B8}, {105, 0x03B9}, {106, 0x03BA},    {107, 0x03BB},
       {108, 0x03BC}, {109, 0x03BD},    {110, 0x03BE}, {111, 0x03BF}, {112, 0x03C0},    {113, 0x03C1},
       {114, 0x03C2}, {115, 0x03C3},    {116, 0x03C4}, {117, 0x03C5}, {118, 0x03C6},    {119, 0x03C7},
       {120, 0x03C8}, {121, 0x03C9},    {122, 0x03CA}, {123, 0x03CB}, {124, 0x03CC},    {125, 0x03CD},
       {126, 0x03CE}, InvalidEntry(127)}};

    template <CodeUnitInputIterator<ISO_8859_7> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_7> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_7, MightBeInvalidEntry(true)>(*current);
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

    // TODO(possible-optimisation): Is padding the array to 128 entries better than using a sparse array of 92
    // entries? The latter would save memory but will be slower to search.
    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {
      {{0, 0x0080},      {1, 0x0081},      {2, 0x0082},      {3, 0x0083},       {4, 0x0084},
       {5, 0x0085},      {6, 0x0086},      {7, 0x0087},      {8, 0x0088},       {9, 0x0089},
       {10, 0x008A},     {11, 0x008B},     {12, 0x008C},     {13, 0x008D},      {14, 0x008E},
       {15, 0x008F},     {16, 0x0090},     {17, 0x0091},     {18, 0x0092},      {19, 0x0093},
       {20, 0x0094},     {21, 0x0095},     {22, 0x0096},     {23, 0x0097},      {24, 0x0098},
       {25, 0x0099},     {26, 0x009A},     {27, 0x009B},     {28, 0x009C},      {29, 0x009D},
       {30, 0x009E},     {31, 0x009F},     {32, 0x00A0},     InvalidEntry(33),  {34, 0x00A2},
       {35, 0x00A3},     {36, 0x00A4},     {37, 0x00A5},     {38, 0x00A6},      {39, 0x00A7},
       {40, 0x00A8},     {41, 0x00A9},     {42, 0x00D7},     {43, 0x00AB},      {44, 0x00AC},
       {45, 0x00AD},     {46, 0x00AE},     {47, 0x00AF},     {48, 0x00B0},      {49, 0x00B1},
       {50, 0x00B2},     {51, 0x00B3},     {52, 0x00B4},     {53, 0x00B5},      {54, 0x00B6},
       {55, 0x00B7},     {56, 0x00B8},     {57, 0x00B9},     {58, 0x00F7},      {59, 0x00BB},
       {60, 0x00BC},     {61, 0x00BD},     {62, 0x00BE},     InvalidEntry(63),  InvalidEntry(64),
       InvalidEntry(65), InvalidEntry(66), InvalidEntry(67), InvalidEntry(68),  InvalidEntry(69),
       InvalidEntry(70), InvalidEntry(71), InvalidEntry(72), InvalidEntry(73),  InvalidEntry(74),
       InvalidEntry(75), InvalidEntry(76), InvalidEntry(77), InvalidEntry(78),  InvalidEntry(79),
       InvalidEntry(80), InvalidEntry(81), InvalidEntry(82), InvalidEntry(83),  InvalidEntry(84),
       InvalidEntry(85), InvalidEntry(86), InvalidEntry(87), InvalidEntry(88),  InvalidEntry(89),
       InvalidEntry(90), InvalidEntry(91), InvalidEntry(92), InvalidEntry(93),  InvalidEntry(94),
       {95, 0x2017},     {96, 0x05D0},     {97, 0x05D1},     {98, 0x05D2},      {99, 0x05D3},
       {100, 0x05D4},    {101, 0x05D5},    {102, 0x05D6},    {103, 0x05D7},     {104, 0x05D8},
       {105, 0x05D9},    {106, 0x05DA},    {107, 0x05DB},    {108, 0x05DC},     {109, 0x05DD},
       {110, 0x05DE},    {111, 0x05DF},    {112, 0x05E0},    {113, 0x05E1},     {114, 0x05E2},
       {115, 0x05E3},    {116, 0x05E4},    {117, 0x05E5},    {118, 0x05E6},     {119, 0x05E7},
       {120, 0x05E8},    {121, 0x05E9},    {122, 0x05EA},    InvalidEntry(123), InvalidEntry(124),
       {125, 0x200E},    {126, 0x200F},    InvalidEntry(127)}};

    template <CodeUnitInputIterator<ISO_8859_8> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_8> DecodeOne(TIterator current,
                                                                          TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_8, MightBeInvalidEntry(true)>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
      {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
      {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
      {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
      {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x0104},  {34, 0x0112},
      {35, 0x0122},  {36, 0x012A},  {37, 0x0128},  {38, 0x0136},  {39, 0x00A7},  {40, 0x013B},  {41, 0x0110},
      {42, 0x0160},  {43, 0x0166},  {44, 0x017D},  {45, 0x00AD},  {46, 0x016A},  {47, 0x014A},  {48, 0x00B0},
      {49, 0x0105},  {50, 0x0113},  {51, 0x0123},  {52, 0x012B},  {53, 0x0129},  {54, 0x0137},  {55, 0x00B7},
      {56, 0x013C},  {57, 0x0111},  {58, 0x0161},  {59, 0x0167},  {60, 0x017E},  {61, 0x2015},  {62, 0x016B},
      {63, 0x014B},  {64, 0x0100},  {65, 0x00C1},  {66, 0x00C2},  {67, 0x00C3},  {68, 0x00C4},  {69, 0x00C5},
      {70, 0x00C6},  {71, 0x012E},  {72, 0x010C},  {73, 0x00C9},  {74, 0x0118},  {75, 0x00CB},  {76, 0x0116},
      {77, 0x00CD},  {78, 0x00CE},  {79, 0x00CF},  {80, 0x00D0},  {81, 0x0145},  {82, 0x014C},  {83, 0x00D3},
      {84, 0x00D4},  {85, 0x00D5},  {86, 0x00D6},  {87, 0x0168},  {88, 0x00D8},  {89, 0x0172},  {90, 0x00DA},
      {91, 0x00DB},  {92, 0x00DC},  {93, 0x00DD},  {94, 0x00DE},  {95, 0x00DF},  {96, 0x0101},  {97, 0x00E1},
      {98, 0x00E2},  {99, 0x00E3},  {100, 0x00E4}, {101, 0x00E5}, {102, 0x00E6}, {103, 0x012F}, {104, 0x010D},
      {105, 0x00E9}, {106, 0x0119}, {107, 0x00EB}, {108, 0x0117}, {109, 0x00ED}, {110, 0x00EE}, {111, 0x00EF},
      {112, 0x00F0}, {113, 0x0146}, {114, 0x014D}, {115, 0x00F3}, {116, 0x00F4}, {117, 0x00F5}, {118, 0x00F6},
      {119, 0x0169}, {120, 0x00F8}, {121, 0x0173}, {122, 0x00FA}, {123, 0x00FB}, {124, 0x00FC}, {125, 0x00FD},
      {126, 0x00FE}, {127, 0x0138},
    }};

    template <CodeUnitInputIterator<ISO_8859_10> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_10> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_10>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
      {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
      {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
      {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
      {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x201D},  {34, 0x00A2},
      {35, 0x00A3},  {36, 0x00A4},  {37, 0x201E},  {38, 0x00A6},  {39, 0x00A7},  {40, 0x00D8},  {41, 0x00A9},
      {42, 0x0156},  {43, 0x00AB},  {44, 0x00AC},  {45, 0x00AD},  {46, 0x00AE},  {47, 0x00C6},  {48, 0x00B0},
      {49, 0x00B1},  {50, 0x00B2},  {51, 0x00B3},  {52, 0x201C},  {53, 0x00B5},  {54, 0x00B6},  {55, 0x00B7},
      {56, 0x00F8},  {57, 0x00B9},  {58, 0x0157},  {59, 0x00BB},  {60, 0x00BC},  {61, 0x00BD},  {62, 0x00BE},
      {63, 0x00E6},  {64, 0x0104},  {65, 0x012E},  {66, 0x0100},  {67, 0x0106},  {68, 0x00C4},  {69, 0x00C5},
      {70, 0x0118},  {71, 0x0112},  {72, 0x010C},  {73, 0x00C9},  {74, 0x0179},  {75, 0x0116},  {76, 0x0122},
      {77, 0x0136},  {78, 0x012A},  {79, 0x013B},  {80, 0x0160},  {81, 0x0143},  {82, 0x0145},  {83, 0x00D3},
      {84, 0x014C},  {85, 0x00D5},  {86, 0x00D6},  {87, 0x00D7},  {88, 0x0172},  {89, 0x0141},  {90, 0x015A},
      {91, 0x016A},  {92, 0x00DC},  {93, 0x017B},  {94, 0x017D},  {95, 0x00DF},  {96, 0x0105},  {97, 0x012F},
      {98, 0x0101},  {99, 0x0107},  {100, 0x00E4}, {101, 0x00E5}, {102, 0x0119}, {103, 0x0113}, {104, 0x010D},
      {105, 0x00E9}, {106, 0x017A}, {107, 0x0117}, {108, 0x0123}, {109, 0x0137}, {110, 0x012B}, {111, 0x013C},
      {112, 0x0161}, {113, 0x0144}, {114, 0x0146}, {115, 0x00F3}, {116, 0x014D}, {117, 0x00F5}, {118, 0x00F6},
      {119, 0x00F7}, {120, 0x0173}, {121, 0x0142}, {122, 0x015B}, {123, 0x016B}, {124, 0x00FC}, {125, 0x017C},
      {126, 0x017E}, {127, 0x2019},
    }};

    template <CodeUnitInputIterator<ISO_8859_13> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_13> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_13>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
      {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
      {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
      {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
      {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x1E02},  {34, 0x1E03},
      {35, 0x00A3},  {36, 0x010A},  {37, 0x010B},  {38, 0x1E0A},  {39, 0x00A7},  {40, 0x1E80},  {41, 0x00A9},
      {42, 0x1E82},  {43, 0x1E0B},  {44, 0x1EF2},  {45, 0x00AD},  {46, 0x00AE},  {47, 0x0178},  {48, 0x1E1E},
      {49, 0x1E1F},  {50, 0x0120},  {51, 0x0121},  {52, 0x1E40},  {53, 0x1E41},  {54, 0x00B6},  {55, 0x1E56},
      {56, 0x1E81},  {57, 0x1E57},  {58, 0x1E83},  {59, 0x1E60},  {60, 0x1EF3},  {61, 0x1E84},  {62, 0x1E85},
      {63, 0x1E61},  {64, 0x00C0},  {65, 0x00C1},  {66, 0x00C2},  {67, 0x00C3},  {68, 0x00C4},  {69, 0x00C5},
      {70, 0x00C6},  {71, 0x00C7},  {72, 0x00C8},  {73, 0x00C9},  {74, 0x00CA},  {75, 0x00CB},  {76, 0x00CC},
      {77, 0x00CD},  {78, 0x00CE},  {79, 0x00CF},  {80, 0x0174},  {81, 0x00D1},  {82, 0x00D2},  {83, 0x00D3},
      {84, 0x00D4},  {85, 0x00D5},  {86, 0x00D6},  {87, 0x1E6A},  {88, 0x00D8},  {89, 0x00D9},  {90, 0x00DA},
      {91, 0x00DB},  {92, 0x00DC},  {93, 0x00DD},  {94, 0x0176},  {95, 0x00DF},  {96, 0x00E0},  {97, 0x00E1},
      {98, 0x00E2},  {99, 0x00E3},  {100, 0x00E4}, {101, 0x00E5}, {102, 0x00E6}, {103, 0x00E7}, {104, 0x00E8},
      {105, 0x00E9}, {106, 0x00EA}, {107, 0x00EB}, {108, 0x00EC}, {109, 0x00ED}, {110, 0x00EE}, {111, 0x00EF},
      {112, 0x0175}, {113, 0x00F1}, {114, 0x00F2}, {115, 0x00F3}, {116, 0x00F4}, {117, 0x00F5}, {118, 0x00F6},
      {119, 0x1E6B}, {120, 0x00F8}, {121, 0x00F9}, {122, 0x00FA}, {123, 0x00FB}, {124, 0x00FC}, {125, 0x00FD},
      {126, 0x0177}, {127, 0x00FF},
    }};

    template <CodeUnitInputIterator<ISO_8859_14> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_14> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_14>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
      {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
      {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
      {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
      {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x00A1},  {34, 0x00A2},
      {35, 0x00A3},  {36, 0x20AC},  {37, 0x00A5},  {38, 0x0160},  {39, 0x00A7},  {40, 0x0161},  {41, 0x00A9},
      {42, 0x00AA},  {43, 0x00AB},  {44, 0x00AC},  {45, 0x00AD},  {46, 0x00AE},  {47, 0x00AF},  {48, 0x00B0},
      {49, 0x00B1},  {50, 0x00B2},  {51, 0x00B3},  {52, 0x017D},  {53, 0x00B5},  {54, 0x00B6},  {55, 0x00B7},
      {56, 0x017E},  {57, 0x00B9},  {58, 0x00BA},  {59, 0x00BB},  {60, 0x0152},  {61, 0x0153},  {62, 0x0178},
      {63, 0x00BF},  {64, 0x00C0},  {65, 0x00C1},  {66, 0x00C2},  {67, 0x00C3},  {68, 0x00C4},  {69, 0x00C5},
      {70, 0x00C6},  {71, 0x00C7},  {72, 0x00C8},  {73, 0x00C9},  {74, 0x00CA},  {75, 0x00CB},  {76, 0x00CC},
      {77, 0x00CD},  {78, 0x00CE},  {79, 0x00CF},  {80, 0x00D0},  {81, 0x00D1},  {82, 0x00D2},  {83, 0x00D3},
      {84, 0x00D4},  {85, 0x00D5},  {86, 0x00D6},  {87, 0x00D7},  {88, 0x00D8},  {89, 0x00D9},  {90, 0x00DA},
      {91, 0x00DB},  {92, 0x00DC},  {93, 0x00DD},  {94, 0x00DE},  {95, 0x00DF},  {96, 0x00E0},  {97, 0x00E1},
      {98, 0x00E2},  {99, 0x00E3},  {100, 0x00E4}, {101, 0x00E5}, {102, 0x00E6}, {103, 0x00E7}, {104, 0x00E8},
      {105, 0x00E9}, {106, 0x00EA}, {107, 0x00EB}, {108, 0x00EC}, {109, 0x00ED}, {110, 0x00EE}, {111, 0x00EF},
      {112, 0x00F0}, {113, 0x00F1}, {114, 0x00F2}, {115, 0x00F3}, {116, 0x00F4}, {117, 0x00F5}, {118, 0x00F6},
      {119, 0x00F7}, {120, 0x00F8}, {121, 0x00F9}, {122, 0x00FA}, {123, 0x00FB}, {124, 0x00FC}, {125, 0x00FD},
      {126, 0x00FE}, {127, 0x00FF},
    }};

    template <CodeUnitInputIterator<ISO_8859_15> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_15> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_15>(*current);
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

    constexpr static Array<EncodingLookupEntry, 128uz> LookupMap = {{
      {0, 0x0080},   {1, 0x0081},   {2, 0x0082},   {3, 0x0083},   {4, 0x0084},   {5, 0x0085},   {6, 0x0086},
      {7, 0x0087},   {8, 0x0088},   {9, 0x0089},   {10, 0x008A},  {11, 0x008B},  {12, 0x008C},  {13, 0x008D},
      {14, 0x008E},  {15, 0x008F},  {16, 0x0090},  {17, 0x0091},  {18, 0x0092},  {19, 0x0093},  {20, 0x0094},
      {21, 0x0095},  {22, 0x0096},  {23, 0x0097},  {24, 0x0098},  {25, 0x0099},  {26, 0x009A},  {27, 0x009B},
      {28, 0x009C},  {29, 0x009D},  {30, 0x009E},  {31, 0x009F},  {32, 0x00A0},  {33, 0x0104},  {34, 0x0105},
      {35, 0x0141},  {36, 0x20AC},  {37, 0x201E},  {38, 0x0160},  {39, 0x00A7},  {40, 0x0161},  {41, 0x00A9},
      {42, 0x0218},  {43, 0x00AB},  {44, 0x0179},  {45, 0x00AD},  {46, 0x017A},  {47, 0x017B},  {48, 0x00B0},
      {49, 0x00B1},  {50, 0x010C},  {51, 0x0142},  {52, 0x017D},  {53, 0x201D},  {54, 0x00B6},  {55, 0x00B7},
      {56, 0x017E},  {57, 0x010D},  {58, 0x0219},  {59, 0x00BB},  {60, 0x0152},  {61, 0x0153},  {62, 0x0178},
      {63, 0x017C},  {64, 0x00C0},  {65, 0x00C1},  {66, 0x00C2},  {67, 0x0102},  {68, 0x00C4},  {69, 0x0106},
      {70, 0x00C6},  {71, 0x00C7},  {72, 0x00C8},  {73, 0x00C9},  {74, 0x00CA},  {75, 0x00CB},  {76, 0x00CC},
      {77, 0x00CD},  {78, 0x00CE},  {79, 0x00CF},  {80, 0x0110},  {81, 0x0143},  {82, 0x00D2},  {83, 0x00D3},
      {84, 0x00D4},  {85, 0x0150},  {86, 0x00D6},  {87, 0x015A},  {88, 0x0170},  {89, 0x00D9},  {90, 0x00DA},
      {91, 0x00DB},  {92, 0x00DC},  {93, 0x0118},  {94, 0x021A},  {95, 0x00DF},  {96, 0x00E0},  {97, 0x00E1},
      {98, 0x00E2},  {99, 0x0103},  {100, 0x00E4}, {101, 0x0107}, {102, 0x00E6}, {103, 0x00E7}, {104, 0x00E8},
      {105, 0x00E9}, {106, 0x00EA}, {107, 0x00EB}, {108, 0x00EC}, {109, 0x00ED}, {110, 0x00EE}, {111, 0x00EF},
      {112, 0x0111}, {113, 0x0144}, {114, 0x00F2}, {115, 0x00F3}, {116, 0x00F4}, {117, 0x0151}, {118, 0x00F6},
      {119, 0x015B}, {120, 0x0171}, {121, 0x00F9}, {122, 0x00FA}, {123, 0x00FB}, {124, 0x00FC}, {125, 0x0119},
      {126, 0x021B}, {127, 0x00FF},
    }};

    template <CodeUnitInputIterator<ISO_8859_16> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ISO_8859_16> DecodeOne(TIterator current,
                                                                           TIterator end) noexcept
    {
      krys_debug_assert(current != end);
      return DecodeOneWithExtendedASCIILookupMap<ISO_8859_16>(*current);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ISO_8859_16> EncodeOne(char32 codePoint) noexcept
    {
      return EncodeOneWithExtendedASCIILookupMap<ISO_8859_16>(codePoint);
    }
  };

  static_assert(Encoding<ISO_8859_16>);

#pragma endregion
}