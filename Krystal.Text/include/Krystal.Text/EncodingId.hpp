#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cstddef>

namespace Krys::Text
{
  enum class EncodingId
  {
    unknown = 0,
    ascii,
    utf7imap,
    utf7,
    utfebcdic,
    utf8,
    mutf8,
    wtf8,
    cesu8,
    utf16,
    utf16le,
    utf16be,
    utf32,
    utf32le,
    utf32be,
    gb18030,
    utf1,
    ucs2,
    ucs4
  };

  KRYS_NODISCARD constexpr inline bool IsUnicodeEncoding(EncodingId id) noexcept
  {
    switch (id)
    {
      case EncodingId::utf7:
      case EncodingId::utf7imap:
      case EncodingId::utfebcdic:
      case EncodingId::utf8:
      case EncodingId::utf16:
      case EncodingId::utf16le:
      case EncodingId::utf16be:
      case EncodingId::utf32:
      case EncodingId::utf32le:
      case EncodingId::utf32be:
      case EncodingId::gb18030:
      case EncodingId::wtf8:
      case EncodingId::mutf8:
      case EncodingId::utf1:
      case EncodingId::cesu8:     return true;
      default:                    return false;
    }
  }

  KRYS_NODISCARD constexpr inline EncodingId ToByteEncoding(EncodingId id, Endian::Type endianness,
                                                            std::size_t characterSize) noexcept
  {
    if (characterSize == sizeof(uchar))
    {
      switch (id)
      {
        case EncodingId::utf7:
        case EncodingId::utf7imap:
        case EncodingId::utfebcdic:
        case EncodingId::utf8:
        case EncodingId::ascii:
        case EncodingId::utf1:
        case EncodingId::cesu8:
        case EncodingId::mutf8:
        case EncodingId::wtf8:
        case EncodingId::gb18030:   return id;
        default:                    break;
      }
    }

    if (endianness != Endian::Big && endianness != Endian::Little)
    {
      return EncodingId::unknown;
    }

    switch (id)
    {
      case EncodingId::utf16: return endianness == Endian::Big ? EncodingId::utf16be : EncodingId::utf16le;
      case EncodingId::utf32: return endianness == Endian::Big ? EncodingId::utf32be : EncodingId::utf32le;
      default:                return EncodingId::unknown;
    }
  }
}
