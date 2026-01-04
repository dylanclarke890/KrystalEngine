#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include "Krystal.Text/TextEncodingId.hpp"

#include <string_view>

namespace Krys
{
  namespace Impl
  {
    template <typename TChar>
    constexpr inline Array<TChar, 63> ReadableCharactersStorage = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0'};
    template <typename TChar>
    constexpr inline Array<TChar, 27> UncasedCharactersStorage = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\0'};
    template <typename TChar>
    constexpr inline Array<TChar, 27> CasedCharactersStorage = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                                                                'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                                                'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\0'};

    template <typename TChar, typename TView = std::basic_string_view<TChar, char_traits_for<TChar>>>
    constexpr inline TView ReadableCharacters {ReadableCharactersStorage<TChar>.data(),
                                               ReadableCharactersStorage<TChar>.size() - 1};
    template <typename TChar, typename TView = std::basic_string_view<TChar, char_traits_for<TChar>>>
    constexpr inline TView UncasedCharacters {UncasedCharactersStorage<TChar>.data(),
                                              UncasedCharactersStorage<TChar>.size() - 1};
    template <typename TChar, typename TView = std::basic_string_view<TChar, char_traits_for<TChar>>>
    constexpr inline TView CasedCharacters {CasedCharactersStorage<TChar>.data(),
                                            CasedCharactersStorage<TChar>.size() - 1};

    constexpr inline Array<const char *, 16> UnicodeNames = {
      "UTF-7",    "UTF-7-IMAP", "UTF-8",        "UTF-16", "UTF-32", "UTF-16LE", "UTF-16BE", "UTF-32LE",
      "UTF-32BE", "UTF-EBCDIC", "UTF-8-EBCDIC", "MUTF-8", "WTF-8",  "GB18030",  "CESU-8",   "UTF-1"};

    constexpr inline std::string_view ToName(TextEncodingId id) noexcept
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
        default:                        return "unknown";
      }
    }

    inline char8 NormalizeASCIILetter(char8 c0)
    {
      if (c0 <= 'Z' && c0 >= 'A')
      {
        // A is equivalent to a, etc. etc.
        // ASCII character, make sure lowercase
        // add to 5th bit (0-based) to get lowercase
        c0 &= static_cast<char8>(0x20);
      }
      return c0;
    }

    template <typename _Target>
    void InplaceIdNormalize(_Target &target)
    {
      // FIXME: full unicode case folding at some point in my short,
      // miserable existence
      // FIXME: full normalization form D case folding at some point in my
      // pathetic mortality
      size_t targetSize = target.size();
      for (size_t i = 0; i < targetSize;)
      {
        char8 c0 = target[i];
        bool isASCII = ((c0 & 0x7F) == 0);
        if (isASCII)
        {
          if (c0 == '-' || c0 == '_' || c0 == ' ' || c0 == '\r' || c0 == '\n' || c0 == '\t')
          {
            // skip dash, underscore, related whitespace
            target.erase(target.begin() + i);
            continue;
          }
          target[i] = NormalizeASCIILetter(c0);
          ++i;
          continue;
        }

        i += 1;
      }
    }
  }

  template <typename TLeft, typename TRight>
  constexpr inline bool IsEncodingNameEqualFor(const TLeft &left, const TRight &right) noexcept
  {
    using TLeftChar = typename TLeft::value_type;
    using TRightChar = typename TRight::value_type;
    std::size_t leftSize = left.size();
    std::size_t rightSize = right.size();
    auto leftPtr = left.data();
    auto rightPtr = right.data();
    std::size_t leftIndex = 0;
    std::size_t rightIndex = 0;
    for (; leftIndex < leftSize && rightIndex < rightSize;)
    {
      // find the first non-ignorable character we can read
      std::size_t leftFirstIndex = left.find_first_of(Impl::ReadableCharacters<TLeftChar, TLeft>, leftIndex);
      if (leftFirstIndex == std::string_view::npos)
      {
        return rightIndex == rightSize;
      }
      leftIndex = leftFirstIndex + 1;
      std::size_t rightFirstIndex =
        right.find_first_of(Impl::ReadableCharacters<TRightChar, TRight>, rightIndex);
      if (rightFirstIndex == std::string_view::npos)
      {
        return leftIndex == leftSize;
      }
      rightIndex = rightFirstIndex + 1;
      auto leftCh = leftPtr[leftFirstIndex];
      auto rightCh = rightPtr[rightFirstIndex];
      // make sure we eliminate casing differences
      std::size_t leftChCasingIndex = Impl::UncasedCharacters<TLeftChar, TLeft>.find(leftCh);
      if (leftChCasingIndex != std::string_view::npos)
      {
        leftCh = Impl::CasedCharacters<TLeftChar>[leftChCasingIndex];
      }
      std::size_t rightChCasingIndex = Impl::UncasedCharacters<TRightChar, TRight>.find(rightCh);
      if (rightChCasingIndex != std::string_view::npos)
      {
        rightCh = Impl::CasedCharacters<TRightChar, TRight>[rightChCasingIndex];
      }
      // finally, check
      if (leftCh == rightCh)
      {
        continue;
      }
      return false;
    }
    bool leftExhausted =
      left.find_first_of(Impl::ReadableCharacters<TLeftChar, TLeft>.data(), leftIndex) == TLeft::npos;
    bool rightExhausted =
      right.find_first_of(Impl::ReadableCharacters<TRightChar, TRight>.data(), rightIndex) == TRight::npos;
    return leftExhausted && rightExhausted;
  }

  constexpr inline bool IsEncodingNameEqual(std::string_view left, std::string_view right) noexcept
  {
    return IsEncodingNameEqualFor(left, right);
  }

  constexpr inline bool IsEncodingNameEqual(std::basic_string_view<char8> left,
                                            std::basic_string_view<char8> right) noexcept
  {
    return IsEncodingNameEqualFor(left, right);
  }

  constexpr inline bool IsUnicodeEncodingName(std::string_view encodingName) noexcept
  {
    for (std::size_t i = 0; i < Impl::UnicodeNames.size(); ++i)
    {
      std::string_view unicodeName = Impl::UnicodeNames[i];
      if (IsEncodingNameEqual(encodingName, unicodeName))
      {
        return true;
      }
    }
    return false;
  }

  constexpr inline bool IsUnicodeEncodingName(std::basic_string_view<char8> encodingName) noexcept
  {
    for (std::size_t i = 0; i < Impl::UnicodeNames.size(); ++i)
    {
      std::string_view unicodeName = Impl::UnicodeNames[i];
      if (IsEncodingNameEqualFor(encodingName, unicodeName))
      {
        return true;
      }
    }
    return false;
  }

  namespace Impl
  {

    constexpr inline TextEncodingId ToEncodingId(std::string_view name)
    {
      if (Krys::IsEncodingNameEqual(name, "UTF-8"))
      {
        return TextEncodingId::utf8;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-16") || Krys::IsEncodingNameEqual(name, "UCS-2-INTERNAL")
               || Krys::IsEncodingNameEqual(name, "UCS-2"))
      {
        return TextEncodingId::utf16;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-16LE")
               || Krys::IsEncodingNameEqual(name, "UCS-2LE-INTERNAL")
               || Krys::IsEncodingNameEqual(name, "UCS-2LE"))
      {
        return TextEncodingId::utf16le;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-16BE")
               || Krys::IsEncodingNameEqual(name, "UCS-2BE-INTERNAL")
               || Krys::IsEncodingNameEqual(name, "UCS-2BE"))
      {
        return TextEncodingId::utf16be;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-32") || Krys::IsEncodingNameEqual(name, "UCS-4-INTERNAL")
               || Krys::IsEncodingNameEqual(name, "UCS-4"))
      {
        return TextEncodingId::utf32;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-32LE")
               || Krys::IsEncodingNameEqual(name, "UCS-4LE-INTERNAL")
               || Krys::IsEncodingNameEqual(name, "UCS-4LE"))
      {
        return TextEncodingId::utf32le;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-32BE")
               || Krys::IsEncodingNameEqual(name, "UCS-4BE-INTERNAL")
               || Krys::IsEncodingNameEqual(name, "UCS-4BE"))
      {
        return TextEncodingId::utf32be;
      }
      else if (Krys::IsEncodingNameEqual(name, "ASCII") || Krys::IsEncodingNameEqual(name, "ANSI_X3.4-1968")
               || Krys::IsEncodingNameEqual(name, "US-ASCII"))
      {
        return TextEncodingId::ascii;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-EBCDIC")
               || Krys::IsEncodingNameEqual(name, "UTF-8-EBCDIC"))
      {
        return TextEncodingId::utfebcdic;
      }
      else if (Krys::IsEncodingNameEqual(name, "WTF-8"))
      {
        return TextEncodingId::wtf8;
      }
      else if (Krys::IsEncodingNameEqual(name, "MUTF-8"))
      {
        return TextEncodingId::mutf8;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-7"))
      {
        return TextEncodingId::utf7;
      }
      else if (Krys::IsEncodingNameEqual(name, "UTF-7-IMAP"))
      {
        return TextEncodingId::utf7imap;
      }
      else
      {
        return TextEncodingId::unknown;
      }
    }

    constexpr inline TextEncodingId ToNormalizedUnicodeEncodingId(std::string_view name)
    {
      const TextEncodingId id = ToEncodingId(name);
      switch (id)
      {
        case TextEncodingId::utf8:    return TextEncodingId::utf8;
        case TextEncodingId::utf16:
        case TextEncodingId::utf16le:
        case TextEncodingId::utf16be: return TextEncodingId::utf16;
        case TextEncodingId::utf32:
        case TextEncodingId::utf32le:
        case TextEncodingId::utf32be: return TextEncodingId::utf32;
        default:                      return TextEncodingId::unknown;
      }
    }

  }
}
