#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::text
{
  /// @brief The unique identifier for a supported text encoding.
  /// @note charsets can be found at https://www.iana.org/assignments/charset-reg
  /// @todo continue adding support for additional encodings; there are plenty more to do from the above link.
  enum class EncodingId : uint8
  {
    Unknown = 0,
    ASCII,
    ATASCII,
    AtariST,
    Big5_HKSCS_2001,
    EUC_KR_UHC,
    GBK,
    GB18030,
    IBM424HebrewBulletin,
    IBM856Hebrew,
    IBM866Cyrillic,
    IBM1006Urdu,
    ISO_8859_1,
    ISO_8859_1_1985,
    ISO_8859_1_1998,
    ISO_8859_2,
    ISO_8859_3,
    ISO_8859_4,
    ISO_8859_5,
    ISO_8859_6,
    ISO_8859_7,
    ISO_8859_8,
    ISO_8859_10,
    ISO_8859_13,
    ISO_8859_14,
    ISO_8859_15,
    ISO_8859_16,
    KAMENICKY,
    KOI8R,
    KOI8U,
    KZ_1048,
    MacCyrillic,
    MacRoman,
    PETSCIIShifted,
    PETSCIIUnshifted,
    ShiftJISx0208,
    TatarANSI,
    TatarASCII,
    UTF8,
    UTF16BE,
    UTF16LE,
    UTF32BE,
    UTF32LE,
    Windows437DOSLatinUS,
    Windows865DOSNordic,
    Windows874,
    Windows1252,
    Windows1251,
    Windows1250,
    Windows1253,
    Windows1254,
    Windows1255,
    Windows1256,
    Windows1257,
    Windows1258,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::text::EncodingId, 55uz)