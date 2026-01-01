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

namespace Krys
{
  template <>
  constexpr underlying_t<UI::NodeType> OrdinalCount<UI::NodeType> = 2;
}