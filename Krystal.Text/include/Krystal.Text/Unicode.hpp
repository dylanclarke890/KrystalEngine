#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/Rune.hpp"

namespace Krys
{
  struct Unicode : NonCopyMovable<Unicode>
  {
    Unicode() = delete;
    ~Unicode() = delete;

    constexpr static uint32 MaxSupplementaryPlaneValue = 0x10FFFFu;
    constexpr static uint32 MaxBasicMultilingualPlaneValue = 0xFFFFu;

    constexpr static uint32 SurrogateHighStart = 0xD800u;
    constexpr static uint32 SurrogateHighEnd = 0xDBFFu;

    constexpr static uint32 SurrogateLowStart = 0xDC00u;
    constexpr static uint32 SurrogateLowEnd = 0xDFFFu;

    /// @brief Determines whether 'codepoint' is a valid Unicode scalar value.
    KRYS_NODISCARD constexpr static bool IsValidCodepoint(uint32 codepoint) noexcept
    {
      return codepoint <= MaxSupplementaryPlaneValue && !IsSurrogateCodepoint(static_cast<uint16>(codepoint));
    }

    /// @brief Determines whether 'codepoint' is a surrogate codepoint.
    KRYS_NODISCARD constexpr static bool IsSurrogateCodepoint(uint16 codepoint) noexcept
    {
      return (codepoint >= SurrogateHighStart && codepoint <= SurrogateLowEnd);
    }

    /// @brief Determines whether 'codepoint' is a high surrogate.
    KRYS_NODISCARD constexpr static bool IsHighSurrogate(uint16 codepoint) noexcept
    {
      return (codepoint >= SurrogateHighStart && codepoint <= SurrogateHighEnd);
    }

    /// @brief Determines whether 'codepoint' is a low surrogate.
    KRYS_NODISCARD constexpr static bool IsLowSurrogate(uint16 codepoint) noexcept
    {
      return (codepoint >= SurrogateLowStart && codepoint <= SurrogateLowEnd);
    }

    /// @brief Determines whether 'rune' is within the basic multilingual plane.
    KRYS_NODISCARD constexpr static bool IsBasicMultilingualPlaneCodepoint(Rune rune) noexcept
    {
      return rune <= MaxBasicMultilingualPlaneValue;
    }

    /// @brief Converts a pair of UTF-16 surrogate code units to a Unicode codepoint.
    KRYS_NODISCARD constexpr static Rune ConvertSurrogatePair(uint16 high, uint16 low) noexcept
    {
      return Rune(0x10000u + ((high - Unicode::SurrogateHighStart) << 10u)
                  + (low - Unicode::SurrogateLowStart));
    }

    /// @brief Encodes 'rune' into its UTF-8 representation and appends it to 'out'.
    template <typename T>
    constexpr static void ToUTF8(Rune rune, T &out) noexcept
    {
      using ValueType = typename T::value_type;

      if (ASCII::IsASCII(rune))
      {
        out.push_back(static_cast<ValueType>(rune.Value));
      }
      else if (rune <= 0x7FFu)
      {
        out.push_back(static_cast<ValueType>(0xC0 | (rune >> 6)));
        out.push_back(static_cast<ValueType>(0x80 | (rune & 0x3F)));
      }
      else if (rune <= MaxBasicMultilingualPlaneValue)
      {
        out.push_back(static_cast<ValueType>(0xE0 | (rune >> 12)));
        out.push_back(static_cast<ValueType>(0x80 | ((rune >> 6) & 0x3F)));
        out.push_back(static_cast<ValueType>(0x80 | (rune & 0x3F)));
      }
      else if (rune <= MaxSupplementaryPlaneValue)
      {
        out.push_back(static_cast<ValueType>(0xF0 | (rune >> 18)));
        out.push_back(static_cast<ValueType>(0x80 | ((rune >> 12) & 0x3F)));
        out.push_back(static_cast<ValueType>(0x80 | ((rune >> 6) & 0x3F)));
        out.push_back(static_cast<ValueType>(0x80 | (rune & 0x3F)));
      }
    }

    /// @brief Determines the number of bytes required to encode the given codepoint in UTF-8.
    KRYS_NODISCARD constexpr static uint8 ByteCount(Rune rune) noexcept
    {
      if (rune <= ASCII::MaxASCIIValue)
      {
        return 1u;
      }
      else if (rune <= 0x7FFu)
      {
        return 2u;
      }
      else if (rune <= Unicode::MaxBasicMultilingualPlaneValue)
      {
        return 3u;
      }
      else
      {
        return 4u;
      }
    }

    /// @brief Attempts to decode the next Unicode codepoint from the given UTF-8 stream.
    template <typename T>
    KRYS_NODISCARD constexpr static bool TryGetNextCodepoint(T input, size_t &offset, Rune &out) noexcept
    {
      if (input.begin() == input.end() || offset >= static_cast<size_t>(input.end() - input.begin()))
      {
        return false;
      }

      auto it = input.begin() + offset;
      uint8 first = static_cast<uint8>(*it++);

      if (ASCII::IsASCII(first))
      {
        out = Rune(first);
        offset += 1;
        return true;
      }

      uint32 codepoint = 0;
      size_t needed = 0;
      if ((first & 0xE0) == 0xC0)
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
        return false; // Invalid first byte
      }

      if (static_cast<size_t>(input.end() - it) < needed)
      {
        return false; // Not enough bytes
      }

      for (size_t i = 0; i < needed; i++)
      {
        uint8 b = static_cast<uint8>(*it);
        if ((b & 0xC0) != 0x80)
        {
          return false; // Malformed continuation byte
        }

        codepoint = (codepoint << 6) | (b & 0x3F);
        it++;
      }

      offset += needed + 1;
      out = Rune(codepoint);
      return true;
    }

    /// @brief Decodes a UTF-8 stream into a list of Unicode codepoints.
    template <typename T>
    KRYS_NODISCARD constexpr static List<Rune> GetCodepoints(T input) noexcept
    {
      List<Rune> codepoints;
      GetCodepoints(input, codepoints);
      return codepoints;
    }

    /// @brief Appends the Unicode codepoints from the given UTF-8 stream to the output list.
    template <typename T>
    constexpr static void GetCodepoints(T input, List<Rune> &out) noexcept
    {
      ForEachCodepoint(input, [&](Rune codepoint) { out.push_back(codepoint); });
    }

    /// @brief Enumerates each Unicode codepoint in the given UTF-8 stream, invoking the provided callable for
    /// each codepoint.
    template <typename T, ForEachCodepointCallable TFunc>
    constexpr static void ForEachCodepoint(T input, TFunc &&func) noexcept
    {
      constexpr auto InvokeFunc = []<typename F>(F &&f, Rune codepoint, bool wasInvalid)
      {
        if constexpr (Callable<F, Rune, bool>)
        {
          f(codepoint, wasInvalid);
        }
        else
        {
          f(codepoint);
        }
      };

      constexpr Rune ReplacementCharacter(0xFFFDu);

      auto it = input.begin();
      while (it != input.end())
      {
        uint8 first = static_cast<uint8>(*it++);
        if (ASCII::IsASCII(first))
        {
          InvokeFunc(func, Rune(first), false);
          continue;
        }

        uint32 codepoint = 0;
        uint8 needed = 0;
        if ((first & 0xE0) == 0xC0)
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
          InvokeFunc(func, ReplacementCharacter, true);
          continue;
        }

        if (static_cast<size_t>(input.end() - it) < needed)
        {
          InvokeFunc(func, ReplacementCharacter, true);
          break;
        }

        for (uint8 i = 0; i < needed; i++)
        {
          uint8 b = static_cast<uint8>(*it);
          if ((b & 0xC0) != 0x80)
          {
            InvokeFunc(func, ReplacementCharacter, true);
            it++; // Advance past the malformed byte
            continue;
          }

          codepoint = (codepoint << 6) | (b & 0x3F);
          it++;
        }

        // Check for invalid encodings
        bool invalidTwoByte = needed == 1u && codepoint < 0x80u;
        bool invalidThreeByte = needed == 2u && codepoint < 0x800u;
        bool invalidFourByte = needed == 3u && codepoint < 0x10000u;
        if (!IsValidCodepoint(codepoint) || invalidTwoByte || invalidThreeByte || invalidFourByte)
        {
          InvokeFunc(func, ReplacementCharacter, true);
          continue;
        }

        InvokeFunc(func, Rune(codepoint), false);
      }
    }
  };
}