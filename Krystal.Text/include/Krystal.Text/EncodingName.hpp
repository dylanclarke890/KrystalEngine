#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/EncodingId.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <string_view>

namespace Krys::Text::detail
{
  template <typename TEncoding>
  concept HasName = requires {
    { TEncoding::Name };
  };

  template <typename TEncoding>
  concept HasAliases = requires {
    { TEncoding::Aliases };
  };
}

namespace Krys::Text
{

  template <typename TEncoding>
  constexpr ASCIILiteral GetEncodingName() noexcept
  {
    if constexpr (::Krys::Text::detail::HasName<TEncoding>)
    {
      return TEncoding::Name;
    }
    else
    {
      return ""_s;
    }
  }

  template <typename TEncoding>
  constexpr Span<const ASCIILiteral> GetEncodingAliases() noexcept
  {
    if constexpr (::Krys::Text::detail::HasAliases<TEncoding>)
    {
      return Span<const ASCIILiteral>(TEncoding::Aliases);
    }
    else
    {
      return Span<const ASCIILiteral>();
    }
  }

  namespace Impl
  {
    template <typename TChar>
    constexpr inline Array<TChar, 63> ReadableCharactersStorage = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0'};

    template <typename TChar, typename TView = std::basic_string_view<TChar, char_traits_for<TChar>>>
    constexpr inline TView ReadableCharacters {ReadableCharactersStorage<TChar>.data(),
                                               ReadableCharactersStorage<TChar>.size() - 1};

    template <typename TChar>
    constexpr inline Array<TChar, 27> UncasedCharactersStorage = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\0'};

    template <typename TChar, typename TView = std::basic_string_view<TChar, char_traits_for<TChar>>>
    constexpr inline TView UncasedCharacters {UncasedCharactersStorage<TChar>.data(),
                                              UncasedCharactersStorage<TChar>.size() - 1};

    template <typename TChar>
    constexpr inline Array<TChar, 27> CasedCharactersStorage = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                                                                'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                                                'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\0'};

    template <typename TChar, typename TView = std::basic_string_view<TChar, char_traits_for<TChar>>>
    constexpr inline TView CasedCharacters {CasedCharactersStorage<TChar>.data(),
                                            CasedCharactersStorage<TChar>.size() - 1};

    constexpr inline Array<const char *, 16> UnicodeNames = {
      "UTF-7",    "UTF-7-IMAP", "UTF-8",        "UTF-16", "UTF-32", "UTF-16LE", "UTF-16BE", "UTF-32LE",
      "UTF-32BE", "UTF-EBCDIC", "UTF-8-EBCDIC", "MUTF-8", "WTF-8",  "GB18030",  "CESU-8",   "UTF-1"};

    constexpr inline std::string_view ToName(EncodingId id) noexcept
    {
      switch (id)
      {
        case EncodingId::unknown:   return "unknown";
        case EncodingId::utf7imap:  return "utf7imap";
        case EncodingId::utf7:      return "utf7";
        case EncodingId::utfebcdic: return "utfebcdic";
        case EncodingId::utf8:      return "utf8";
        case EncodingId::mutf8:     return "mutf8";
        case EncodingId::wtf8:      return "wtf8";
        case EncodingId::utf16:     return "utf16";
        case EncodingId::utf16le:   return "utf16le";
        case EncodingId::utf16be:   return "utf16be";
        case EncodingId::utf32:     return "utf32";
        case EncodingId::utf32le:   return "utf32le";
        case EncodingId::utf32be:   return "utf32be";
        case EncodingId::gb18030:   return "gb18030";
        case EncodingId::utf1:      return "utf1";
        case EncodingId::cesu8:     return "cesu8";
        case EncodingId::ascii:     return "ascii";
        default:                    return "unknown";
      }
    }

    inline char8 NormalizeASCIILetter(char8 c0)
    {
      if (c0 <= 'Z' && c0 >= 'A')
      {
        // A is equivalent to a, etc. etc.
        // ASCII character, make sure lowercase - add to 5th bit (0-Based)
        c0 &= static_cast<char8>(0x20);
      }
      return c0;
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
    for (std::size_t i = 0; i < ::Krys::Text::Impl::UnicodeNames.size(); ++i)
    {
      std::string_view unicodeName = ::Krys::Text::Impl::UnicodeNames[i];
      if (IsEncodingNameEqual(encodingName, unicodeName))
      {
        return true;
      }
    }
    return false;
  }

  constexpr inline bool IsUnicodeEncodingName(std::basic_string_view<char8> encodingName) noexcept
  {
    for (std::size_t i = 0; i < ::Krys::Text::Impl::UnicodeNames.size(); ++i)
    {
      std::string_view unicodeName = ::Krys::Text::Impl::UnicodeNames[i];
      if (IsEncodingNameEqualFor(encodingName, unicodeName))
      {
        return true;
      }
    }
    return false;
  }

  namespace Impl
  {

    constexpr inline EncodingId ToEncodingId(std::string_view name)
    {
      if (Krys::Text::IsEncodingNameEqual(name, "UTF-8"))
      {
        return EncodingId::utf8;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-16")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-2-INTERNAL")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-2"))
      {
        return EncodingId::utf16;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-16LE")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-2LE-INTERNAL")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-2LE"))
      {
        return EncodingId::utf16le;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-16BE")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-2BE-INTERNAL")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-2BE"))
      {
        return EncodingId::utf16be;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-32")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-4-INTERNAL")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-4"))
      {
        return EncodingId::utf32;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-32LE")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-4LE-INTERNAL")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-4LE"))
      {
        return EncodingId::utf32le;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-32BE")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-4BE-INTERNAL")
               || Krys::Text::IsEncodingNameEqual(name, "UCS-4BE"))
      {
        return EncodingId::utf32be;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "ASCII")
               || Krys::Text::IsEncodingNameEqual(name, "ANSI_X3.4-1968")
               || Krys::Text::IsEncodingNameEqual(name, "US-ASCII"))
      {
        return EncodingId::ascii;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-EBCDIC")
               || Krys::Text::IsEncodingNameEqual(name, "UTF-8-EBCDIC"))
      {
        return EncodingId::utfebcdic;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "WTF-8"))
      {
        return EncodingId::wtf8;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "MUTF-8"))
      {
        return EncodingId::mutf8;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-7"))
      {
        return EncodingId::utf7;
      }
      else if (Krys::Text::IsEncodingNameEqual(name, "UTF-7-IMAP"))
      {
        return EncodingId::utf7imap;
      }
      else
      {
        return EncodingId::unknown;
      }
    }

    constexpr inline EncodingId ToNormalizedUnicodeEncodingId(std::string_view name)
    {
      const EncodingId id = ToEncodingId(name);
      switch (id)
      {
        case EncodingId::utf8:    return EncodingId::utf8;
        case EncodingId::utf16:
        case EncodingId::utf16le:
        case EncodingId::utf16be: return EncodingId::utf16;
        case EncodingId::utf32:
        case EncodingId::utf32le:
        case EncodingId::utf32be: return EncodingId::utf32;
        default:                  return EncodingId::unknown;
      }
    }

  }
}
