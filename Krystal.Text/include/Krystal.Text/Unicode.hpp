#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::Text
{
  /// @brief Represents a Unicode codepoint.
  struct UnicodeCodepoint : public StronglyTypedNumber<UnicodeCodepoint, uint32>
  {
    using StronglyTypedNumber<UnicodeCodepoint, uint32>::StronglyTypedNumber;
  };

  template <typename T>
  concept UnicodeCodepointCallable = Callable<T, UnicodeCodepoint> || Callable<T, UnicodeCodepoint, bool>;

  struct Unicode : NonCopyMovable<Unicode>
  {
    Unicode() = delete;
    ~Unicode() = delete;

    constexpr static uint32 DefaultReplacementCodepoint = 0xFFFDu;

    constexpr static uint8 MaxASCIIValue = 0x7Fu;
    constexpr static uint32 MaxSupplementaryPlaneValue = 0x10FFFFu;
    constexpr static uint32 MaxBasicMultilingualPlaneValue = 0xFFFFu;
    constexpr static uint8 ExtendedASCIIStart = Unicode::MaxASCIIValue + 1u;

    constexpr static uint32 SurrogateHighStart = 0xD800u;
    constexpr static uint32 SurrogateHighEnd = 0xDBFFu;
    constexpr static uint32 SurrogateLowStart = 0xDC00u;
    constexpr static uint32 SurrogateLowEnd = 0xDFFFu;

    /// @brief Determines whether the given codepoint is an ASCII character.
    KRYS_NODISCARD constexpr static bool IsASCIICharacter(UnicodeCodepoint ch) noexcept
    {
      return ch <= MaxASCIIValue;
    }

    /// @brief Determines whether the given char is an ASCII character.
    KRYS_NODISCARD constexpr static bool IsASCIICharacter(uint8 ch) noexcept
    {
      return IsASCIICharacter(UnicodeCodepoint(ch));
    }

    /// @brief Determines whether the given byte is an ASCII character.
    KRYS_NODISCARD constexpr static bool IsASCIICharacter(byte ch) noexcept
    {
      return IsASCIICharacter(UnicodeCodepoint(static_cast<uint8>(ch)));
    }

    /// @brief Determines whether the given UTF-8 string consists entirely of ASCII characters.
    KRYS_NODISCARD constexpr static bool IsASCIIString(utf8_stringview str) noexcept
    {
      for (char8_t ch : str)
      {
        if (!IsASCIICharacter(static_cast<uint8>(ch)))
        {
          return false;
        }
      }
      return true;
    }

    /// @brief Determines whether the given codepoint is a valid Unicode scalar value.
    KRYS_NODISCARD constexpr static bool IsValidCodepoint(UnicodeCodepoint codepoint) noexcept
    {
      return (codepoint <= MaxSupplementaryPlaneValue) && !IsSurrogateCodepoint(codepoint);
    }

    KRYS_NODISCARD constexpr static bool
      IsBasicMultilingualPlaneCodepoint(UnicodeCodepoint codepoint) noexcept
    {
      return codepoint <= MaxBasicMultilingualPlaneValue;
    }

    /// @brief Determines whether the given codepoint is a surrogate codepoint.
    KRYS_NODISCARD constexpr static bool IsSurrogateCodepoint(UnicodeCodepoint codepoint) noexcept
    {
      return (codepoint >= SurrogateHighStart && codepoint <= SurrogateLowEnd);
    }

    /// @brief Determines whether the given codepoint is a high surrogate.
    KRYS_NODISCARD constexpr static bool IsHighSurrogate(UnicodeCodepoint codepoint) noexcept
    {
      return (codepoint >= SurrogateHighStart && codepoint <= SurrogateHighEnd);
    }

    /// @brief Determines whether the given codepoint is a low surrogate.
    KRYS_NODISCARD constexpr static bool IsLowSurrogate(UnicodeCodepoint codepoint) noexcept
    {
      return (codepoint >= SurrogateLowStart && codepoint <= SurrogateLowEnd);
    }

    /// @brief Encodes a single Unicode codepoint into its UTF-8 representation and appends it to the output
    /// string.
    constexpr static void CodepointToUTF8(UnicodeCodepoint codepoint, utf8_string &out) noexcept
    {
      constexpr uint32 MaxTwoByteValue = 0x7FFu;

      if (codepoint <= MaxASCIIValue)
      {
        out.push_back(static_cast<char8_t>(codepoint));
      }
      else if (codepoint <= MaxTwoByteValue)
      {
        out.push_back(static_cast<char8_t>(0xC0 | (codepoint >> 6)));
        out.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
      else if (codepoint <= MaxBasicMultilingualPlaneValue)
      {
        out.push_back(static_cast<char8_t>(0xE0 | (codepoint >> 12)));
        out.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
      else if (codepoint <= MaxSupplementaryPlaneValue)
      {
        out.push_back(static_cast<char8_t>(0xF0 | (codepoint >> 18)));
        out.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
    }

    /// @brief Encodes a single Unicode codepoint into its UTF-8 representation and appends it to the output
    /// bytes.
    constexpr static void CodepointToUTF8(UnicodeCodepoint codepoint, List<byte> &out) noexcept
    {
      constexpr uint32 MaxTwoByteValue = 0x7FFu;

      if (codepoint <= MaxASCIIValue)
      {
        out.push_back(static_cast<byte>(codepoint.Value));
      }
      else if (codepoint <= MaxTwoByteValue)
      {
        out.push_back(static_cast<byte>(0xC0 | (codepoint >> 6)));
        out.push_back(static_cast<byte>(0x80 | (codepoint & 0x3F)));
      }
      else if (codepoint <= MaxBasicMultilingualPlaneValue)
      {
        out.push_back(static_cast<byte>(0xE0 | (codepoint >> 12)));
        out.push_back(static_cast<byte>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<byte>(0x80 | (codepoint & 0x3F)));
      }
      else if (codepoint <= MaxSupplementaryPlaneValue)
      {
        out.push_back(static_cast<byte>(0xF0 | (codepoint >> 18)));
        out.push_back(static_cast<byte>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<byte>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<byte>(0x80 | (codepoint & 0x3F)));
      }
    }

    /// @brief Enumerates each Unicode codepoint in the given UTF-8 string, invoking the provided callable for
    /// each codepoint.
    template <UnicodeCodepointCallable TFunc>
    constexpr static void ForEachCodepoint(utf8_stringview str, TFunc &&func) noexcept
    {
      auto begin = str.begin();
      const auto end = str.end();
      ForEachCodepointImpl(std::forward<TFunc>(func), begin, end);
    }

    /// @brief Enumerates each Unicode codepoint in the given UTF-8 byte stream, invoking the provided
    /// callable for each codepoint.
    template <UnicodeCodepointCallable TFunc>
    constexpr static void ForEachCodepoint(Span<const byte> bytes, TFunc &&func) noexcept
    {
      auto begin = bytes.begin();
      const auto end = bytes.end();
      ForEachCodepointImpl(std::forward<TFunc>(func), begin, end);
    }

    /// @brief Decodes a UTF-8 string into a list of Unicode codepoints.
    KRYS_NODISCARD constexpr static List<UnicodeCodepoint> GetCodepoints(utf8_stringview str) noexcept
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
        out.reserve(out.size() + str.size());
      }

      const auto appendScalar = [&](UnicodeCodepoint codepoint) noexcept
      {
        out.push_back(codepoint);
      };

      ForEachCodepoint(str, appendScalar);
    }

    /// @brief Encodes a list of Unicode codepoints into a UTF-8 string.
    KRYS_NODISCARD constexpr static utf8_string CodepointsToUTF8(Span<UnicodeCodepoint> codepoints) noexcept
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
        out.reserve(out.size() + codepoints.size());
      }

      for (UnicodeCodepoint codepoint : codepoints)
      {
        CodepointToUTF8(codepoint, out);
      }
    }

  private:
    template <UnicodeCodepointCallable TFunc, typename TIteratorStart, typename TIteratorEnd>
    constexpr static void ForEachCodepointImpl(TFunc &&func, TIteratorStart begin, TIteratorEnd end) noexcept
    {
      constexpr UnicodeCodepoint REPLACEMENT_CHARACTER(DefaultReplacementCodepoint);

      constexpr auto InvokeFunc = []<typename F>(F &&f, UnicodeCodepoint codepoint, bool wasInvalid)
      {
        if constexpr (Callable<F, UnicodeCodepoint, bool>)
        {
          f(codepoint, wasInvalid);
        }
        else
        {
          f(codepoint);
        }
      };

      auto it = begin;
      while (it != end)
      {
        uint32 codepoint = 0;
        uint8 needed = 0;

        uint8 first = static_cast<uint8>(*it++);
        if (IsASCIICharacter(first))
        {
          InvokeFunc(func, UnicodeCodepoint(first), false);
          continue;
        }
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
        else
        {
          InvokeFunc(func, REPLACEMENT_CHARACTER, true);
          continue;
        }

        if (static_cast<size_t>(end - it) < needed)
        {
          InvokeFunc(func, REPLACEMENT_CHARACTER, true);
          break;
        }

        bool malformed = false;
        for (uint8 i = 0; i < needed; i++)
        {
          uint8 b = static_cast<uint8>(*it);
          if ((b & 0xC0) != 0x80)
          {
            InvokeFunc(func, REPLACEMENT_CHARACTER, true);
            malformed = true;
            break;
          }

          codepoint = (codepoint << 6) | (b & 0x3F);
          it++;
        }

        if (malformed)
        {
          continue;
        }

        // Invalid encodings
        bool invalidTwoByte = needed == 1u && codepoint < 0x80u;
        bool invalidThreeByte = needed == 2u && codepoint < 0x800u;
        bool invalidFourByte = needed == 3u && codepoint < 0x10000u;
        bool invalidCodepoint = codepoint > MaxSupplementaryPlaneValue;
        bool invalidSurrogate = IsSurrogateCodepoint(UnicodeCodepoint(codepoint));
        if (invalidTwoByte || invalidThreeByte || invalidFourByte || invalidCodepoint || invalidSurrogate)
        {
          InvokeFunc(func, REPLACEMENT_CHARACTER, true);
          continue;
        }

        InvokeFunc(func, UnicodeCodepoint(codepoint), false);
      }
    }
  };
}