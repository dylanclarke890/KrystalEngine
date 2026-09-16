#pragma once

#include "Krystal.Core/Concepts.hpp"
#include <string>
#include <string_view>

namespace krys
{
  using string = std::string;
  using utf8_string = std::u8string;
  using utf16_string = std::u16string;
  using utf32_string = std::u32string;

  using stringview = std::string_view;
  using utf8_stringview = std::u8string_view;
  using utf16_stringview = std::u16string_view;
  using utf32_stringview = std::u32string_view;

  template <typename T>
  concept IsStringView = OneOf<T, stringview, utf8_stringview, utf16_stringview, utf32_stringview>;

  template <typename T>
  concept IsString = OneOf<T, string, utf8_string, utf16_string, utf32_string>;
}