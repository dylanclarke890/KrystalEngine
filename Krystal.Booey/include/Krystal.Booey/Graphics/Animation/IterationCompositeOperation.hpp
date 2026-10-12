#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  enum class IterationCompositeOperation : uint8
  {
    Replace,
    Accumulate
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::IterationCompositeOperation, 2uz);
