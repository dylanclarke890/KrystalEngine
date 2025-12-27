#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Detection.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>

namespace Krys::Text
{
  class LineEndings
  {
    STATIC_CLASS(LineEndings)

  public:
    constexpr static byte LF = byte {'\n'};
    constexpr static byte CR = byte {'\r'};

    /// @brief Normalizes line endings in the given byte stream to LF.
    NO_DISCARD constexpr static List<byte> NormalizeToLF(List<byte> &&bytes) noexcept
    {
      size_t inputIndex = 0;
      size_t outputIndex = 0;

      while (inputIndex < bytes.size())
      {
        const byte character = bytes[inputIndex++];
        if (character == CR) // Turn CRLF and CR into LF.
        {
          if (inputIndex < bytes.size() && bytes[inputIndex] == LF)
          {
            inputIndex++;
          }
          bytes[outputIndex++] = LF;
        }
        else // Leave other characters alone.
        {
          bytes[outputIndex++] = character;
        }
      }

      bytes.resize(outputIndex);
    }

    /// @brief Normalizes line endings in the given byte stream to CRLF.
    NO_DISCARD constexpr static List<byte> NormalizeToCRLF(List<byte> &&bytes) noexcept
    {
      size_t sourceIndex = 0;
      size_t resultLength = 0;
      while (sourceIndex < bytes.size())
      {
        const byte character = bytes[sourceIndex++];
        if (character == CR) // Turn CR or CRLF into CRLF;
        {
          if (sourceIndex < bytes.size() && bytes[sourceIndex] == LF)
          {
            sourceIndex++;
          }
          resultLength += 2;
        }
        else if (character == LF) // Turn LF into CRLF.
        {
          resultLength += 2;
        }
        else // Leave other characters alone.
        {
          resultLength += 1;
        }
      }

      if (resultLength == bytes.size())
      {
        return bytes;
      }

      sourceIndex = 0;
      size_t resultIndex = 0;
      List<byte> result(resultLength);
      while (sourceIndex < bytes.size())
      {
        const byte character = bytes[sourceIndex++];
        if (character == CR) // Turn CR or CRLF into CRLF;
        {
          if (sourceIndex < bytes.size() && bytes[sourceIndex] == LF)
          {
            sourceIndex++;
          }
          result[resultIndex++] = CR;
          result[resultIndex++] = LF;
        }
        else if (character == LF) // Turn LF into CRLF.
        {
          result[resultIndex++] = CR;
          result[resultIndex++] = LF;
        }
        else // Leave other characters alone.
        {
          result[resultIndex++] = character;
        }
      }

      assert(resultIndex == resultLength);
      return result;
    }

    /// @brief Normalizes line endings in the given byte stream to the native format.
    NO_DISCARD constexpr static List<byte> NormalizeToNative(List<byte> &&from) noexcept
    {
#ifdef KRYS_PLATFORM_WINDOWS
      return NormalizeToCRLF(std::forward<List<byte>>(from));
#else
      return NormalizeToLF(std::forward<List<byte>>(from));
#endif
    }
  };
}