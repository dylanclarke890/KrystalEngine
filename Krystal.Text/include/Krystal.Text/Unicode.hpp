#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/StronglyTypedValue.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Text
{
  /// @brief Represents a Unicode codepoint.
  struct UnicodeCodepoint : public StronglyTypedNumber<UnicodeCodepoint, uint32>
  {
    using StronglyTypedNumber<UnicodeCodepoint, uint32>::StronglyTypedNumber;
  };

  struct Unicode
  {
    STATIC_CLASS(Unicode)

    constexpr static uint8 MaxASCIIValue = 0x7Fu;
    constexpr static uint32 MaxCodepointValue = 0x10FFFFu;
    constexpr static uint32 DefaultReplacementCodepoint = 0xFFFDu;

    constexpr static uint32 SurrogateHighStart = 0xD800u;
    constexpr static uint32 SurrogateHighEnd = 0xDBFFu;
    constexpr static uint32 SurrogateLowStart = 0xDC00u;
    constexpr static uint32 SurrogateLowEnd = 0xDFFFu;

    // TODO: there's a lot of overlap in the two ForEachCodepoint methods; refactor to reduce duplication

    /// @brief Enumerates each Unicode codepoint in the given UTF-8 string, invoking the provided callable for
    /// each codepoint.
    template <typename TFunc>
    requires Callable<TFunc, UnicodeCodepoint>
    constexpr static void ForEachCodepoint(utf8_stringview str, TFunc &&func) noexcept
    {
      constexpr UnicodeCodepoint REPLACEMENT_CHARACTER(DefaultReplacementCodepoint);

      auto it = str.begin();
      const auto end = str.end();
      while (it != end)
      {
        uint32 codepoint = 0;
        uint8 needed = 0;

        uint8 first = static_cast<uint8>(*it++);
        // Single-byte (ASCII)
        if (first <= MaxASCIIValue)
        {
          func(UnicodeCodepoint(first));
          continue;
        }
        // Multi-byte sequence
        else if ((first & 0xE0) == 0xC0)
        {
          codepoint = first & 0x1F;
          needed = 1;
        }
        else if ((first & 0xF0) == 0xE0)
        {
          codepoint = first & 0x0F;
          needed = 2;
        }
        else if ((first & 0xF8) == 0xF0)
        {
          codepoint = first & 0x07;
          needed = 3;
        }
        // Invalid leading byte
        else
        {
          func(REPLACEMENT_CHARACTER);
          continue;
        }

        if (static_cast<size_t>(end - it) < needed)
        {
          func(REPLACEMENT_CHARACTER);
          break;
        }

        // Process continuation bytes
        for (uint8 i = 0; i < needed; i++)
        {
          uint8 b = static_cast<uint8>(*it);
          if ((b & 0xC0) != 0x80)
          {
            func(REPLACEMENT_CHARACTER);
            // TODO: test this case
            it++; // sync to next byte
            continue;
          }

          // Append bits
          codepoint = (codepoint << 6) | (b & 0x3F);
          it++;
        }

        // Reject overlong encodings
        bool invalidTwoByte = (needed == 1 && codepoint < 0x80);
        bool invalidThreeByte = (needed == 2 && codepoint < 0x800);
        bool invalidFourByte = (needed == 3 && codepoint < 0x10000);
        bool invalidCodepoint = (codepoint > 0x10FFFF);
        bool invalidSurrogate = (codepoint >= 0xD800 && codepoint <= 0xDFFF);
        if (invalidTwoByte || invalidThreeByte || invalidFourByte || invalidCodepoint || invalidSurrogate)
        {
          func(REPLACEMENT_CHARACTER);
          continue;
        }

        // Valid codepoint
        func(UnicodeCodepoint(codepoint));
      }
    }

    /// @brief Enumerates each Unicode codepoint in the given UTF-8 string, invoking the provided callable for
    /// each codepoint.
    template <typename TFunc>
    requires Callable<TFunc, UnicodeCodepoint>
    constexpr static void ForEachCodepoint(Span<const byte> bytes, TFunc &&func) noexcept
    {
      constexpr UnicodeCodepoint REPLACEMENT_CHARACTER(DefaultReplacementCodepoint);

      auto it = bytes.begin();
      const auto end = bytes.end();
      while (it != end)
      {
        uint32 codepoint = 0;
        uint8 needed = 0;

        uint8 first = static_cast<uint8>(*it++);
        // Single-byte (ASCII)
        if (first <= MaxASCIIValue)
        {
          func(UnicodeCodepoint(first));
          continue;
        }
        // Multi-byte sequence
        else if ((first & 0xE0) == 0xC0)
        {
          codepoint = first & 0x1F;
          needed = 1;
        }
        else if ((first & 0xF0) == 0xE0)
        {
          codepoint = first & 0x0F;
          needed = 2;
        }
        else if ((first & 0xF8) == 0xF0)
        {
          codepoint = first & 0x07;
          needed = 3;
        }
        // Invalid leading byte
        else
        {
          func(REPLACEMENT_CHARACTER);
          continue;
        }

        if (static_cast<size_t>(end - it) < needed)
        {
          func(REPLACEMENT_CHARACTER);
          break;
        }

        bool malformed = false;
        for (uint8 i = 0; i < needed; i++)
        {
          uint8 b = static_cast<uint8>(*it);
          if ((b & 0xC0) != 0x80)
          {
            func(REPLACEMENT_CHARACTER);
            malformed = true;
            break;
          }

          // Append bits
          codepoint = (codepoint << 6) | (b & 0x3F);
          it++;
        }

        if (malformed)
        {
          continue; // abort this UTF-8 sequence
        }

        // Reject overlong encodings
        bool invalidTwoByte = (needed == 1 && codepoint < 0x80);
        bool invalidThreeByte = (needed == 2 && codepoint < 0x800);
        bool invalidFourByte = (needed == 3 && codepoint < 0x10000);
        bool invalidCodepoint = (codepoint > 0x10FFFF);
        bool invalidSurrogate = (codepoint >= 0xD800 && codepoint <= 0xDFFF);
        if (invalidTwoByte || invalidThreeByte || invalidFourByte || invalidCodepoint || invalidSurrogate)
        {
          func(REPLACEMENT_CHARACTER);
          continue;
        }

        // Valid codepoint
        func(UnicodeCodepoint(codepoint));
      }
    }

    /// @brief Decodes a UTF-8 string into a list of Unicode codepoints.
    NO_DISCARD constexpr static List<UnicodeCodepoint> GetCodepoints(utf8_stringview str) noexcept
    {
      List<UnicodeCodepoint> codepoints;
      GetCodepoints(str, codepoints);
      return codepoints;
    }

    /// @brief Appends the Unicode codepoints from the given UTF-8 string to the output list.
    constexpr static void GetCodepoints(utf8_stringview str, List<UnicodeCodepoint> &out) noexcept
    {
      if (out.capacity() < out.size() + str.size())
      {
        out.reserve(out.size() + str.size()); // Conservative estimate, may be larger than actual
      }

      const auto appendScalar = [&](UnicodeCodepoint cp) noexcept
      {
        out.push_back(cp);
      };
      ForEachCodepoint(str, appendScalar);
    }

    /// @brief Encodes a single Unicode codepoint into its UTF-8 representation and appends it to the output
    /// string.
    constexpr static void CodepointToUTF8(UnicodeCodepoint codepoint, utf8_string &out) noexcept
    {
      if (codepoint <= 0x7F)
      {
        out.push_back(static_cast<char8_t>(codepoint));
      }
      else if (codepoint <= 0x7FF)
      {
        out.push_back(static_cast<char8_t>(0xC0 | (codepoint >> 6)));
        out.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
      else if (codepoint <= 0xFFFF)
      {
        out.push_back(static_cast<char8_t>(0xE0 | (codepoint >> 12)));
        out.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
      else if (codepoint <= 0x10FFFF)
      {
        out.push_back(static_cast<char8_t>(0xF0 | (codepoint >> 18)));
        out.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
    }

    /// @brief Encodes a list of Unicode codepoints into a UTF-8 string.
    NO_DISCARD constexpr static utf8_string CodepointsToUTF8(Span<UnicodeCodepoint> codepoints) noexcept
    {
      utf8_string str;
      CodepointsToUTF8(codepoints, str);
      return str;
    }

    /// @brief Appends the UTF-8 encoded representation of the given Unicode codepoints to the output string.
    constexpr static void CodepointsToUTF8(Span<UnicodeCodepoint> codepoints, utf8_string &out) noexcept
    {
      if (out.capacity() < out.size() + codepoints.size())
      {
        out.reserve(out.size() + codepoints.size()); // Conservative estimate, may be larger than actual
      }

      for (UnicodeCodepoint cp : codepoints)
      {
        CodepointToUTF8(cp, out);
      }
    }

    /// @brief Determines whether the given codepoint is a valid Unicode scalar value.
    NO_DISCARD constexpr static bool IsValidCodepoint(UnicodeCodepoint codepoint) noexcept
    {
      if (codepoint > MaxCodepointValue)
      {
        return false;
      }
      if (codepoint >= SurrogateHighStart && codepoint <= SurrogateLowEnd)
      {
        return false;
      }
      return true;
    }

    /// @brief Determines whether the given codepoint is a surrogate codepoint.
    NO_DISCARD constexpr static bool IsSurrogateCodepoint(UnicodeCodepoint codepoint) noexcept
    {
      return (codepoint >= SurrogateHighStart && codepoint <= SurrogateLowEnd);
    }

    /// @brief Determines whether the given codepoint is a high surrogate.
    NO_DISCARD constexpr static bool IsHighSurrogate(UnicodeCodepoint codepoint) noexcept
    {
      return (codepoint >= SurrogateHighStart && codepoint <= SurrogateHighEnd);
    }

    /// @brief Determines whether the given codepoint is a low surrogate.
    NO_DISCARD constexpr static bool IsLowSurrogate(UnicodeCodepoint codepoint) noexcept
    {
      return (codepoint >= SurrogateLowStart && codepoint <= SurrogateLowEnd);
    }

    /// @brief Determines whether the given codepoint is an ASCII character.
    NO_DISCARD constexpr static bool IsACIICharacter(UnicodeCodepoint ch) noexcept
    {
      return ch <= MaxASCIIValue;
    }

    /// @brief Determines whether the given byte is an ASCII character.
    NO_DISCARD constexpr static bool IsACIICharacter(byte b) noexcept
    {
      return static_cast<uint8>(b) <= MaxASCIIValue;
    }

    /// @brief Determines whether the given UTF-8 string consists entirely of ASCII characters.
    NO_DISCARD constexpr static bool IsASCII(utf8_stringview str) noexcept
    {
      bool isASCII = true;

      const auto CheckASCII = [&](UnicodeCodepoint ch) noexcept
      {
        if (!IsACIICharacter(ch))
        {
          isASCII = false;
        }
      };

      ForEachCodepoint(str, CheckASCII);

      return isASCII;
    }
  };
}