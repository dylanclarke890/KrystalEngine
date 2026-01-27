#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Text/Decode/Decode.hpp"
#include "Krystal.Text/Encode/Encode.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"

namespace Krys::Text
{
  KRYS_NODISCARD constexpr utf32_string ConvertToUTF32(utf8_stringview input) noexcept
  {
    return Decode<utf32_string>(input, utf8);
  }

  KRYS_NODISCARD constexpr utf32_string ConvertToUTF32(Span<const char8> input) noexcept
  {
    return Decode<utf32_string>(input, utf8);
  }

  KRYS_NODISCARD constexpr utf8_string ConvertToUTF8(utf32_stringview input) noexcept
  {
    return Encode<utf8_string>(input, utf8);
  }

  KRYS_NODISCARD constexpr utf8_string ConvertToUTF8(Span<const char32> input) noexcept
  {
    return Encode<utf8_string>(input, utf8);
  }
}