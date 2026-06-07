#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class HTMLTokenType : uint8
  {
    Uninitialized,
    DOCTYPE,
    StartTag,
    EndTag,
    Comment,
    Character,
    EndOfFile,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::HTMLTokenType, 7uz);