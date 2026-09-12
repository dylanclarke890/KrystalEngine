#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::HTMLTokenType, 7uz);