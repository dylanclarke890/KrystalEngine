#include "Krystal.Core/Text/Base64.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys
{
  namespace
  {
    constexpr uint8 NonAlphabet = 0xFFu;

    KRYS_NODISCARD constexpr uint8 DecodeCharacter(uint8 c) noexcept
    {
      if (c >= 'A' && c <= 'Z')
      {
        return static_cast<uint8>(c - 'A');
      }
      if (c >= 'a' && c <= 'z')
      {
        return static_cast<uint8>(c - 'a' + 26);
      }
      if (c >= '0' && c <= '9')
      {
        return static_cast<uint8>(c - '0' + 52);
      }
      if (c == '+')
      {
        return 62;
      }
      if (c == '/')
      {
        return 63;
      }
      return NonAlphabet;
    }
  }

  Maybe<List<uint8>> Base64Decode(Span<const uint8> input, EnumSet<Base64DecodeOption> options) noexcept
  {
    List<uint8> destination;
    destination.reserve(input.size());

    size_t equalsSignCount = 0;
    for (const uint8 c : input)
    {
      if (c == '=')
      {
        ++equalsSignCount;
        // There should never be more than 2 padding characters.
        if (options.Contains(Base64DecodeOption::ValidatePadding) && equalsSignCount > 2)
        {
          return null;
        }
        continue;
      }

      const uint8 decoded = DecodeCharacter(c);
      if (decoded != NonAlphabet)
      {
        // Nothing may follow the padding.
        if (equalsSignCount)
        {
          return null;
        }
        destination.push_back(decoded);
      }
      else if (!options.Contains(Base64DecodeOption::IgnoreWhitespace) || !text::IsASCIIWhitespace(c))
      {
        return null;
      }
    }

    size_t outLength = destination.size();
    if (!outLength)
    {
      if (equalsSignCount)
      {
        return null;
      }
      return List<uint8> {};
    }

    // There should be no padding if the length is a multiple of 4. The decoded characters and the padding
    // are counted, rather than the input, so that ignored whitespace doesn't count.
    if (options.Contains(Base64DecodeOption::ValidatePadding) && equalsSignCount
        && (outLength + equalsSignCount) % 4)
    {
      return null;
    }

    // Valid data is (n * 4 + [0,2,3]) characters long.
    if ((outLength % 4) == 1)
    {
      return null;
    }

    // 4-byte to 3-byte conversion.
    const size_t decodedLength = outLength - (outLength + 3) / 4;
    if (!decodedLength)
    {
      return null;
    }

    List<uint8> result(decodedLength);
    size_t sidx = 0;
    size_t didx = 0;
    if (decodedLength > 1)
    {
      while (didx < decodedLength - 2)
      {
        result[didx] = static_cast<uint8>((destination[sidx] << 2) | ((destination[sidx + 1] >> 4) & 0x03u));
        result[didx + 1] =
          static_cast<uint8>((destination[sidx + 1] << 4) | ((destination[sidx + 2] >> 2) & 0x0Fu));
        result[didx + 2] = static_cast<uint8>((destination[sidx + 2] << 6) | (destination[sidx + 3] & 0x3Fu));
        sidx += 4;
        didx += 3;
      }
    }

    if (didx < decodedLength)
    {
      result[didx] = static_cast<uint8>((destination[sidx] << 2) | ((destination[sidx + 1] >> 4) & 0x03u));
    }

    if (++didx < decodedLength)
    {
      result[didx] =
        static_cast<uint8>((destination[sidx + 1] << 4) | ((destination[sidx + 2] >> 2) & 0x0Fu));
    }

    return result;
  }

  utf8_string Base64Encode(Span<const uint8> input) noexcept
  {
    constexpr char8 alphabet[] = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    utf8_string result;
    result.reserve((input.size() + 2) / 3 * 4);
    size_t i = 0;
    for (; i + 2 < input.size(); i += 3)
    {
      uint32 group = (uint32 {input[i]} << 16) | (uint32 {input[i + 1]} << 8) | input[i + 2];
      result += alphabet[(group >> 18) & 0x3Fu];
      result += alphabet[(group >> 12) & 0x3Fu];
      result += alphabet[(group >> 6) & 0x3Fu];
      result += alphabet[group & 0x3Fu];
    }

    if (i + 1 == input.size())
    {
      uint32 group = uint32 {input[i]} << 16;
      result += alphabet[(group >> 18) & 0x3Fu];
      result += alphabet[(group >> 12) & 0x3Fu];
      result += u8"==";
    }
    else if (i + 2 == input.size())
    {
      uint32 group = (uint32 {input[i]} << 16) | (uint32 {input[i + 1]} << 8);
      result += alphabet[(group >> 18) & 0x3Fu];
      result += alphabet[(group >> 12) & 0x3Fu];
      result += alphabet[(group >> 6) & 0x3Fu];
      result += u8'=';
    }
    return result;
  }
}
