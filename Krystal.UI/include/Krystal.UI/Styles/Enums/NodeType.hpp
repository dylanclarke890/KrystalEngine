#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class NodeType : uint8
  {
    Default,
    Text
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::NodeType, 2u)
