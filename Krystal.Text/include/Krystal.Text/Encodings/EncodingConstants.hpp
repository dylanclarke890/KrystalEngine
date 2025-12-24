#pragma once

#include "Krystal.Lib/String/String.hpp"

namespace Krys::Text
{
  constexpr static utf8_stringview EncodingReplacement_ASCII = u8"?";
  constexpr static utf8_stringview IANAName_ASCII = u8"US-ASCII";

  constexpr static utf8_stringview EncodingReplacement_UTF = u8"�";
  constexpr static utf8_stringview IANAName_UTF8 = u8"UTF-8";
  constexpr static utf8_stringview IANAName_UTF16_BE = u8"UTF-16BE";
  constexpr static utf8_stringview IANAName_UTF16_LE = u8"UTF-16LE";
  constexpr static utf8_stringview IANAName_UTF32_BE = u8"UTF-32BE";
  constexpr static utf8_stringview IANAName_UTF32_LE = u8"UTF-32LE";
}