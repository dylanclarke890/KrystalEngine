#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  enum class CompositeOperation : uint8
  {
    Replace,
    Add,
    Accumulate
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::CompositeOperation, 3uz);
