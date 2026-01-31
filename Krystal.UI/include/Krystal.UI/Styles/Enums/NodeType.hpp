#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class NodeType : uint8
  {
    Default,
    Text
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::UI::NodeType, 2u)
