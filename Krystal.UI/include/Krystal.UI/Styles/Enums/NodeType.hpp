#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class NodeType : uint8
  {
    Default,
    Text
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::NodeType>()
  {
    return 2;
  }
}