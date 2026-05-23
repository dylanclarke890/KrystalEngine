#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include <string>
#include <string_view>

namespace Krys
{
  using string = std::string;
  using stringview = std::string_view;

  using utf8_string = std::u8string;
  using utf8_stringview = std::u8string_view;

  using utf16_string = std::u16string;
  using utf16_stringview = std::u16string_view;

  using utf32_string = std::u32string;
  using utf32_stringview = std::u32string_view;

  template <typename T>
  concept IsString =
    SameType<T, string> || SameType<T, utf8_string> || SameType<T, utf16_string> || SameType<T, utf32_string>;
}