// =============================================================================
//
// ztd.idk
// Copyright © JeanHeyd "ThePhD" Meneide and Shepherd's Oasis, LLC
// Contact: opensource@soasis.org
//
// Commercial License Usage
// Licensees holding valid commercial ztd.idk licenses may use this file in
// accordance with the commercial license agreement provided with the
// Software or, alternatively, in accordance with the terms contained in
// a written agreement between you and Shepherd's Oasis, LLC.
// For licensing terms and conditions see your agreement. For
// further information contact opensource@soasis.org.
//
// Apache License Version 2 Usage
// Alternatively, this file may be used under the terms of Apache License
// Version 2.0 (the "License") for non-commercial use; you may not use this
// file except in compliance with the License. You may obtain a copy of the
// License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ============================================================================ //

#pragma once

#ifndef ZTD_IDK_ENCODING_NAME_HPP
  #define ZTD_IDK_ENCODING_NAME_HPP

  #include <ztd/idk/version.hpp>

  #include "Krystal.Text/CharTraits.hpp"
  #include "Krystal.Lib/Types/Numeric.hpp"
  #include "Krystal.Text/TextEncodingId.hpp"

  #include <array>
  #include <string_view>

  

namespace Krys
{
  

  namespace __idk_detail
  {
    template <typename _Char>
    inline constexpr std::array<_Char, 63> __readable_characters_storage_v = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0'};
    template <typename _Char>
    inline constexpr std::array<_Char, 27> __uncased_characters_storage_v = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\0'};
    template <typename _Char>
    inline constexpr std::array<_Char, 27> __cased_characters_storage_v = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
      'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\0'};

    template <typename _Char, typename _View = std::basic_string_view<_Char, ::ztd::char_traits_for<_Char>>>
    inline constexpr _View __readable_characters_v(__readable_characters_storage_v<_Char>.data(),
                                                   __readable_characters_storage_v<_Char>.size() - 1);
    template <typename _Char, typename _View = std::basic_string_view<_Char, ::ztd::char_traits_for<_Char>>>
    inline constexpr _View __uncased_characters_v(__uncased_characters_storage_v<_Char>.data(),
                                                  __uncased_characters_storage_v<_Char>.size() - 1);
    template <typename _Char, typename _View = std::basic_string_view<_Char, ::ztd::char_traits_for<_Char>>>
    inline constexpr _View __cased_characters_v(__cased_characters_storage_v<_Char>.data(),
                                                __cased_characters_storage_v<_Char>.size() - 1);

    inline constexpr const char *__unicode_names[] = {
      "UTF-7",    "UTF-7-IMAP", "UTF-8",        "UTF-16", "UTF-32", "UTF-16LE", "UTF-16BE", "UTF-32LE",
      "UTF-32BE", "UTF-EBCDIC", "UTF-8-EBCDIC", "MUTF-8", "WTF-8",  "GB18030",  "CESU-8",   "UTF-1"};
    inline constexpr std::size_t __unicode_names_count =
      sizeof(__unicode_names) / sizeof(__unicode_names[0]);

    inline constexpr std::string_view to_name(TextEncodingId id) noexcept
    {
      switch (id)
      {
        case TextEncodingId::unknown:   return "unknown";
        case TextEncodingId::utf7imap:  return "utf7imap";
        case TextEncodingId::utf7:      return "utf7";
        case TextEncodingId::utfebcdic: return "utfebcdic";
        case TextEncodingId::utf8:      return "utf8";
        case TextEncodingId::mutf8:     return "mutf8";
        case TextEncodingId::wtf8:      return "wtf8";
        case TextEncodingId::utf16:     return "utf16";
        case TextEncodingId::utf16le:   return "utf16le";
        case TextEncodingId::utf16be:   return "utf16be";
        case TextEncodingId::utf32:     return "utf32";
        case TextEncodingId::utf32le:   return "utf32le";
        case TextEncodingId::utf32be:   return "utf32be";
        case TextEncodingId::gb18030:   return "gb18030";
        case TextEncodingId::utf1:      return "utf1";
        case TextEncodingId::cesu8:     return "cesu8";
        case TextEncodingId::ascii:     return "ascii";
        default:                          return "unknown";
      }
    }

    inline char8 __normalize_ascii_letter(char8 __c0)
    {
      if (__c0 <= 'Z' && __c0 >= 'A')
      {
        // A is equivalent to a, etc. etc.
        // ASCII character, make sure lowercase
        // add to 5th bit (0-based) to get lowercase
        __c0 &= static_cast<char8>(0x20);
      }
      return __c0;
    }

    template <typename _Target>
    void __inplace_id_normalize(_Target &__target)
    {
      // FIXME: full unicode case folding at some point in my short,
      // miserable existence
      // FIXME: full normalization form D case folding at some point in my
      // pathetic mortality
      size_t __target_size = __target.size();
      for (size_t __idx = 0; __idx < __target_size;)
      {
        char8 __c0 = __target[__idx];
        bool __is_ascii = ((__c0 & 0x7F) == 0);
        if (__is_ascii)
        {
          if (__c0 == '-' || __c0 == '_' || __c0 == ' ' || __c0 == '\r' || __c0 == '\n' || __c0 == '\t')
          {
            // skip dash, underscore, related whitespace
            __target.erase(__target.begin() + __idx);
            continue;
          }
          __target[__idx] = __normalize_ascii_letter(__c0);
          ++__idx;
          continue;
        }

        __idx += 1;
      }
    }
  } // namespace __idk_detail

  template <typename TLeft, typename TRight>
  inline constexpr bool is_encoding_name_equal_for(const TLeft &left, const TRight &right) noexcept
  {
    using _LeftChar = typename TLeft::value_type;
    using _RightChar = typename TRight::value_type;
    std::size_t __left_size = left.size();
    std::size_t __right_size = right.size();
    auto __left_ptr = left.data();
    auto __right_ptr = right.data();
    std::size_t __left_index = 0;
    std::size_t __right_index = 0;
    for (; __left_index < __left_size && __right_index < __right_size;)
    {
      // find the first non-ignorable character we can read
      std::size_t __left_first_index =
        left.find_first_of(__idk_detail::__readable_characters_v<_LeftChar, TLeft>, __left_index);
      if (__left_first_index == std::string_view::npos)
      {
        return __right_index == __right_size;
      }
      __left_index = __left_first_index + 1;
      std::size_t __right_first_index =
        right.find_first_of(__idk_detail::__readable_characters_v<_RightChar, TRight>, __right_index);
      if (__right_first_index == std::string_view::npos)
      {
        return __left_index == __left_size;
      }
      __right_index = __right_first_index + 1;
      auto __left_c = __left_ptr[__left_first_index];
      auto __right_c = __right_ptr[__right_first_index];
      // make sure we eliminate casing differences
      std::size_t __left_c_casing_index =
        __idk_detail::__uncased_characters_v<_LeftChar, TLeft>.find(__left_c);
      if (__left_c_casing_index != std::string_view::npos)
      {
        __left_c = __idk_detail::__cased_characters_v<_LeftChar>[__left_c_casing_index];
      }
      std::size_t __right_c_casing_index =
        __idk_detail::__uncased_characters_v<_RightChar, TRight>.find(__right_c);
      if (__right_c_casing_index != std::string_view::npos)
      {
        __right_c = __idk_detail::__cased_characters_v<_RightChar, TRight>[__right_c_casing_index];
      }
      // finally, check
      if (__left_c == __right_c)
      {
        continue;
      }
      return false;
    }
    bool __left_exhausted =
      left.find_first_of(__idk_detail::__readable_characters_v<_LeftChar, TLeft>.data(), __left_index)
      == TLeft::npos;
    bool __right_exhausted =
      right.find_first_of(__idk_detail::__readable_characters_v<_RightChar, TRight>.data(), __right_index)
      == TRight::npos;
    return __left_exhausted && __right_exhausted;
  }

  inline constexpr bool is_encoding_name_equal(std::string_view left, std::string_view right) noexcept
  {
    return is_encoding_name_equal_for(left, right);
  }

  inline constexpr bool is_encoding_name_equal(std::basic_string_view<char8> left,
                                               std::basic_string_view<char8> right) noexcept
  {
    return is_encoding_name_equal_for(left, right);
  }

  inline constexpr bool is_unicode_encoding_name(std::string_view __encoding_name) noexcept
  {
    for (std::size_t i = 0; i < __idk_detail::__unicode_names_count; ++i)
    {
      std::string_view __unicode_name = __idk_detail::__unicode_names[i];
      if (is_encoding_name_equal(__encoding_name, __unicode_name))
      {
        return true;
      }
    }
    return false;
  }

  inline constexpr bool
    is_unicode_encoding_name(std::basic_string_view<char8> __encoding_name) noexcept
  {
    for (std::size_t i = 0; i < __idk_detail::__unicode_names_count; ++i)
    {
      std::string_view __unicode_name = __idk_detail::__unicode_names[i];
      if (is_encoding_name_equal_for(__encoding_name, __unicode_name))
      {
        return true;
      }
    }
    return false;
  }

  namespace __idk_detail
  {

    inline constexpr TextEncodingId __to_encoding_id(std::string_view __name)
    {
      if (::ztd::is_encoding_name_equal(__name, "UTF-8"))
      {
        return TextEncodingId::utf8;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-16")
               || ::ztd::is_encoding_name_equal(__name, "UCS-2-INTERNAL")
               || ::ztd::is_encoding_name_equal(__name, "UCS-2"))
      {
        return TextEncodingId::utf16;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-16LE")
               || ::ztd::is_encoding_name_equal(__name, "UCS-2LE-INTERNAL")
               || ::ztd::is_encoding_name_equal(__name, "UCS-2LE"))
      {
        return TextEncodingId::utf16le;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-16BE")
               || ::ztd::is_encoding_name_equal(__name, "UCS-2BE-INTERNAL")
               || ::ztd::is_encoding_name_equal(__name, "UCS-2BE"))
      {
        return TextEncodingId::utf16be;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-32")
               || ::ztd::is_encoding_name_equal(__name, "UCS-4-INTERNAL")
               || ::ztd::is_encoding_name_equal(__name, "UCS-4"))
      {
        return TextEncodingId::utf32;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-32LE")
               || ::ztd::is_encoding_name_equal(__name, "UCS-4LE-INTERNAL")
               || ::ztd::is_encoding_name_equal(__name, "UCS-4LE"))
      {
        return TextEncodingId::utf32le;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-32BE")
               || ::ztd::is_encoding_name_equal(__name, "UCS-4BE-INTERNAL")
               || ::ztd::is_encoding_name_equal(__name, "UCS-4BE"))
      {
        return TextEncodingId::utf32be;
      }
      else if (::ztd::is_encoding_name_equal(__name, "ASCII")
               || ::ztd::is_encoding_name_equal(__name, "ANSI_X3.4-1968")
               || ::ztd::is_encoding_name_equal(__name, "US-ASCII"))
      {
        return TextEncodingId::ascii;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-EBCDIC")
               || ::ztd::is_encoding_name_equal(__name, "UTF-8-EBCDIC"))
      {
        return TextEncodingId::utfebcdic;
      }
      else if (::ztd::is_encoding_name_equal(__name, "WTF-8"))
      {
        return TextEncodingId::wtf8;
      }
      else if (::ztd::is_encoding_name_equal(__name, "MUTF-8"))
      {
        return TextEncodingId::mutf8;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-7"))
      {
        return TextEncodingId::utf7;
      }
      else if (::ztd::is_encoding_name_equal(__name, "UTF-7-IMAP"))
      {
        return TextEncodingId::utf7imap;
      }
      else
      {
        return TextEncodingId::unknown;
      }
    }

    inline constexpr TextEncodingId __to_normalized_unicode_encoding_id(std::string_view __name)
    {
      const TextEncodingId id = __to_encoding_id(__name);
      switch (id)
      {
        case TextEncodingId::utf8:    return TextEncodingId::utf8;
        case TextEncodingId::utf16:
        case TextEncodingId::utf16le:
        case TextEncodingId::utf16be: return TextEncodingId::utf16;
        case TextEncodingId::utf32:
        case TextEncodingId::utf32le:
        case TextEncodingId::utf32be: return TextEncodingId::utf32;
        default:                        return TextEncodingId::unknown;
      }
    }

  } // namespace __idk_detail

  
} 

  

#endif