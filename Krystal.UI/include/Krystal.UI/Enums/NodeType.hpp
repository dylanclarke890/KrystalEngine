#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class NodeType
  {
    Default,
    Text
  };
}

namespace Krys
{
  template <>
  constexpr int32 OrdinalCount<UI::NodeType>()
  {
    return 2;
  }
}