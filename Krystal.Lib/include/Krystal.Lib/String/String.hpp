#pragma once

#include <string>
#include <string_view>

namespace Krys
{
  using string = std::string;
  using utf8_string = std::u8string;
  using wstring = std::wstring;
  using stringview = std::string_view;
  using utf8_stringview = std::u8string_view;
}