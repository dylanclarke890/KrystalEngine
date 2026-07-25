#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include <catch_all.hpp>
#include <format>
#include <fstream>

namespace Krys::HTML::Tests
{
  KRYS_NODISCARD inline DOMString ToUTF8(const string &s) noexcept
  {
    return utf8_string(s.begin(), s.end());
  };

  template <Number T>
  KRYS_NODISCARD utf8_string ToUTF8(T number) noexcept
  {
    if constexpr (FloatingPoint<T>)
    {
      // NOTE: sorry future me if this causes confusion; doubles are annoying to print and compare.
      auto str = std::format("{:.4f}", number);

      // remove trailing zeroes and the decimal point if necessary
      auto lastNonZero = str.find_last_not_of('0');
      if (lastNonZero != std::string::npos && str[lastNonZero] == '.')
      {
        lastNonZero--;
      }

      if (lastNonZero != std::string::npos)
      {
        str.erase(lastNonZero + 1uz);
      }

      return ToUTF8(str);
    }
    else
    {
      auto str = std::format("{}", number);
      return ToUTF8(str);
    }
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
    char bom[3uz];

    file.read(bom, 3);

    if (file.gcount() == 3 && static_cast<uchar>(bom[0uz]) == 0xEF && static_cast<uchar>(bom[1uz]) == 0xBB
        && static_cast<uchar>(bom[2uz]) == 0xBF)
    {
      // BOM consumed; continue reading
    }
    else
    {
      file.clear();  // clear eof/fail if we hit it
      file.seekg(0); // rewind to beginning
    }
  }

  template <typename ParseFunc>
  KRYS_NODISCARD inline auto ParseTestData(const string &filePath, ParseFunc &&func) noexcept
    -> Maybe<decltype(func(std::declval<std::istream &>()))>
  {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
      return Null;
    }

    SkipUTF8FileBOM(file);
    return func(file);
  }

#define EXECUTE_TEST_CASE(name, parseFile, doTests, rootDir, testDataFile)                                   \
  TEST_CASE(name "(" testDataFile ")", "[HTML][" name "]")                                                   \
  {                                                                                                          \
    auto tests = ParseTestData(rootDir##testDataFile, &parseFile);                                           \
    REQUIRE(tests.has_value());                                                                              \
    doTests(*tests);                                                                                         \
  }

#define UTF8_INFO(str) INFO(string(reinterpret_cast<const char *>(str.data()), str.size()));
}