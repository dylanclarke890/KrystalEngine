#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/EncodingName.hpp"
#include "Krystal.Text/Encodings/ASCII.hpp"
#include "Krystal.Text/Encodings/NoEncoding.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <string_view>

namespace Krys::Impl
{
  template <typename TChar, TextEncodingId TId>
  KRYS_NODISCARD constexpr auto SelectCompileTimeEncoding()
  {
    if constexpr (TId == TextEncodingId::utf8)
    {
      return basic_utf8<TChar> {};
    }
    else if constexpr (TId == TextEncodingId::mutf8)
    {
      return basic_mutf8<TChar> {};
    }
    else if constexpr (TId == TextEncodingId::wtf8)
    {
      return basic_wtf8<TChar> {};
    }
    else if constexpr (TId == TextEncodingId::utf16)
    {
      return basic_utf16<TChar> {};
    }
    else if constexpr (TId == TextEncodingId::utf16le)
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
    else if constexpr (TId == TextEncodingId::utf16be)
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
    else if constexpr (TId == TextEncodingId::utf32)
    {
      return basic_utf32<TChar> {};
    }
    else if constexpr (TId == TextEncodingId::utf32le)
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
    else if constexpr (TId == TextEncodingId::utf32be)
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
    else if constexpr (TId == TextEncodingId::ascii)
    {
      return basic_ascii<TChar> {};
    }
    else
    {
      return basic_no_encoding<TChar, UnicodeCodePoint> {};
    }
  }
}
