#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include <catch_all.hpp>
#include <format>
#include <fstream>

namespace Krys::HTML::Tests
{
  KRYS_NODISCARD inline DOMString ToUTF8(const string &s) noexcept
  {
    return DOMString(reinterpret_cast<const char8_t *>(s.data()), s.size());
  };

  template <Number T>
  KRYS_NODISCARD utf8_string ToUTF8(T number) noexcept
  {
    auto str = std::format("{}", number);
    return utf8_string(str.begin(), str.end());
  }

  template <typename T>
  inline void NormaliseData(T &str) noexcept
  {
    if (!str.empty() && str.back() == U'\n')
    {
      str.pop_back();
    }
  }

  inline void Indent(DOMString &output, size_t depth, DOMString prefix = u8"| ") noexcept
  {
    output += prefix;

    for (size_t i = 0uz; i < depth; ++i)
    {
      output += u8"  ";
    }
  }

  inline void SkipUTF8FileBOM(std::ifstream &file)
  {
    char bom[3];

    file.read(bom, 3);

    if (file.gcount() == 3 && static_cast<uchar>(bom[0]) == 0xEF && static_cast<uchar>(bom[1]) == 0xBB
        && static_cast<uchar>(bom[2]) == 0xBF)
    {
      // BOM consumed; continue reading
    }
    else
    {
      file.clear();  // clear eof/fail if we hit it
      file.seekg(0); // rewind to beginning
    }
  }

  inline void UTF8Info(const utf8_string &str) noexcept
  {
    INFO(string(reinterpret_cast<const char *>(str.data()), str.size()));
  }

#define UTF8_INFO(str) INFO(string(reinterpret_cast<const char *>(str.data()), str.size()));
}