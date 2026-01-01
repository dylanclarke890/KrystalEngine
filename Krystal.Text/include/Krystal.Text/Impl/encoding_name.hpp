#pragma once

#include <ztd/text/ascii.hpp>
#include <ztd/text/encoding_scheme.hpp>
#include <ztd/text/iconv_names.hpp>
#include <ztd/text/no_encoding.hpp>
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/EncodingName.hpp"

#include <string_view>

namespace Krys
{
  namespace Impl
  {
    template <typename TChar, TextEncodingId TId>
    constexpr auto __select_compile_time_encoding()
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
        if constexpr (::ztd::endian::native == ::ztd::endian::little)
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
        if constexpr (::ztd::endian::native == ::ztd::endian::big)
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
        if constexpr (::ztd::endian::native == ::ztd::endian::little)
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
        if constexpr (::ztd::endian::native == ::ztd::endian::big)
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

    template <typename TCodePoint>
    c_string_view __platform_utf_name()
    {
      constexpr std::size_t __bits = sizeof(TCodePoint) * CHAR_BIT;
      if constexpr (__bits <= 8)
      {
        return iconv_utf8_name;
      }
      else if constexpr (__bits <= 16)
      {
        return iconv_utf16_name;
      }
      else if constexpr (__bits <= 32)
      {
        return iconv_utf32_name;
      }
      else
      {
        static_assert(
          always_false_v<TCodePoint>,
          "[ztd.text] There is no good known default platform encoding for the given bit size in this "
          "platform. File a bug and tell us about your needs!");
        return iconv_locale_name;
      }
    }

  }
}
