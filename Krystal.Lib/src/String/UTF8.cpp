#include "Krystal.Lib/String/UTF8.hpp"

namespace Krys
{
  List<Codepoint> UTF8::Decode(const utf8_stringview &str) noexcept
  {
    List<Codepoint> codepoints;
    auto it = str.begin();

    while (it != str.end())
    {
      uint8 first = static_cast<uint8>(*it++);
      uint32 codepoint = 0;
      size_t numBytes = 0;

      if (first < 0x80)
      {
        codepoint = first;
        numBytes = 0;
      }
      else if ((first & 0xE0) == 0xC0)
      {
        codepoint = first & 0x1F;
        numBytes = 1;
      }
      else if ((first & 0xF0) == 0xE0)
      {
        codepoint = first & 0x0F;
        numBytes = 2;
      }
      else if ((first & 0xF8) == 0xF0)
      {
        codepoint = first & 0x07;
        numBytes = 3;
      }
      else
      {
        // Invalid UTF-8 start byte
        continue;
      }

      for (size_t i = 0; i < numBytes && it != str.end(); ++i)
      {
        uint8 byte = static_cast<uint8>(*it++);
        if ((byte & 0xC0) != 0x80) // must start with 10xxxxxx
          break;
        codepoint = (codepoint << 6) | (byte & 0x3F);
      }

      codepoints.emplace_back(Codepoint(codepoint));
    }

    return codepoints;
  }

  utf8_string UTF8::Encode(const List<Codepoint> &codepoints) noexcept
  {
    utf8_string result;
    for (const auto &cp : codepoints)
    {
      uint32 codepoint = static_cast<uint32>(cp);
      if (codepoint <= 0x7F)
      {
        result.push_back(static_cast<char8_t>(codepoint));
      }
      else if (codepoint <= 0x7FF)
      {
        result.push_back(static_cast<char8_t>(0xC0 | (codepoint >> 6)));
        result.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
      else if (codepoint <= 0xFFFF)
      {
        result.push_back(static_cast<char8_t>(0xE0 | (codepoint >> 12)));
        result.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
      else if (codepoint <= 0x10FFFF)
      {
        result.push_back(static_cast<char8_t>(0xF0 | (codepoint >> 18)));
        result.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 12) & 0x3F)));
        result.push_back(static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char8_t>(0x80 | (codepoint & 0x3F)));
      }
    }
    return result;
  }
}