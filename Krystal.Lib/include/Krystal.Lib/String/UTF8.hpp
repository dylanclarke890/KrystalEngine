#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String/Codepoint.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys
{
  class UTF8
  {
    STATIC_CLASS(UTF8)

  public:
    NO_DISCARD static List<Codepoint> Decode(const utf8_stringview &str) noexcept;

    NO_DISCARD static utf8_string Encode(const List<Codepoint> &codepoints) noexcept;
  };
}