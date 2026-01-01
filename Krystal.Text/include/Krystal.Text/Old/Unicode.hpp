#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/Old/ASCII.hpp"
#include "Krystal.Text/Old/Rune.hpp"

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

    constexpr static Rune ReplacementCharacter = Rune(0xFFFDu);

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
    KRYS_NODISCARD constexpr static void EncodeSurrogatePair(Rune character, uint16 &high,
                                                             uint16 &low) noexcept
    {
      uint32 surrogate = character.Value - 0x10000;
      high = static_cast<uint16>(SurrogateHighStart + (surrogate >> 10));
      low = static_cast<uint16>(SurrogateLowStart + (surrogate & 0x3FF));
    }

    /// @brief Converts a pair of UTF-16 surrogate code units to a Unicode codepoint.
    KRYS_NODISCARD constexpr static Rune DecodeSurrogatePair(uint16 high, uint16 low) noexcept
    {
      return Rune(0x10000u + ((high - SurrogateHighStart) << 10u) + (low - SurrogateLowStart));
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
      constexpr auto InvokeFunc = []<typename F>(F &&f, Rune codepoint, bool replaced)
      {
        if constexpr (Callable<F, Rune, bool>)
        {
          f(codepoint, replaced);
        }
        else
        {
          f(codepoint);
        }
      };

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
          InvokeFunc(func, Rune(ReplacementCharacter), true);
          continue;
        }

        if (static_cast<size_t>(input.end() - it) < needed)
        {
          InvokeFunc(func, Rune(ReplacementCharacter), true);
          break;
        }

        for (uint8 i = 0; i < needed; i++)
        {
          uint8 b = static_cast<uint8>(*it);
          if ((b & 0xC0) != 0x80)
          {
            InvokeFunc(func, Rune(ReplacementCharacter), true);
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
          InvokeFunc(func, Rune(ReplacementCharacter), true);
          continue;
        }

        InvokeFunc(func, Rune(codepoint), false);
      }
    }
  };
}