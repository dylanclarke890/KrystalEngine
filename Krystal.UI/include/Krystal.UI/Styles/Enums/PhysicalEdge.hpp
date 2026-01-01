#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Utils/ToUnderlying.hpp"
#include "Krystal.UI/Styles/Enums/Edge.hpp"
#include <utility>

namespace Krys::UI
{
  enum class PhysicalEdge : uint32
  {
    Left = ToUnderlying(Edge::Left),
    Top = ToUnderlying(Edge::Top),
    Right = ToUnderlying(Edge::Right),
    Bottom = ToUnderlying(Edge::Bottom),
  };
}