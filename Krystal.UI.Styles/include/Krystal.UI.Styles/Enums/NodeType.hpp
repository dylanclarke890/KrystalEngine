#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
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
  constexpr uint32 OrdinalCount<UI::Styles::NodeType>()
  {
    return 2;
  }
}