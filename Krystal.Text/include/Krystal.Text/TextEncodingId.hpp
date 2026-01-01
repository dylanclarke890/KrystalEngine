#pragma once

#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cstddef>

namespace Krys
{
  enum class TextEncodingId
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

  inline constexpr bool is_unicode_encoding_id(TextEncodingId id) noexcept
  {
    switch (id)
    {
      case TextEncodingId::utf7:
      case TextEncodingId::utf7imap:
      case TextEncodingId::utfebcdic:
      case TextEncodingId::utf8:
      case TextEncodingId::utf16:
      case TextEncodingId::utf16le:
      case TextEncodingId::utf16be:
      case TextEncodingId::utf32:
      case TextEncodingId::utf32le:
      case TextEncodingId::utf32be:
      case TextEncodingId::gb18030:
      case TextEncodingId::wtf8:
      case TextEncodingId::mutf8:
      case TextEncodingId::utf1:
      case TextEncodingId::cesu8:     return true;
      case TextEncodingId::ascii:
      case TextEncodingId::unknown:
      default:                        return false;
    }
  }

  inline constexpr TextEncodingId to_byte_text_encoding_id(TextEncodingId id, Endian::Type endianness,
                                                           std::size_t characterSize) noexcept
  {
    if (characterSize == sizeof(uchar))
    {
      switch (id)
      {
        case TextEncodingId::utf7:
        case TextEncodingId::utf7imap:
        case TextEncodingId::utfebcdic:
        case TextEncodingId::utf8:
        case TextEncodingId::ascii:
        case TextEncodingId::utf1:
        case TextEncodingId::cesu8:
        case TextEncodingId::mutf8:
        case TextEncodingId::wtf8:
        case TextEncodingId::gb18030:   return id;
        default:                        break;
      }
    }
    switch (id)
    {
      case TextEncodingId::utf16:
        return (endianness == Endian::Big
                  ? TextEncodingId::utf16be
                  : (endianness == Endian::Little ? TextEncodingId::utf16le : TextEncodingId::unknown));
      case TextEncodingId::utf32:
        return (endianness == Endian::Big
                  ? TextEncodingId::utf32be
                  : (endianness == Endian::Little ? TextEncodingId::utf32le : TextEncodingId::unknown));
      default: return TextEncodingId::unknown;
    }
  }
}
