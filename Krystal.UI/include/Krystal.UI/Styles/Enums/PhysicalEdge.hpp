#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Utils/ToUnderlying.hpp"
#include "Krystal.UI/Styles/Enums/Edge.hpp"
#include <utility>

namespace krys::UI
{
  enum class PhysicalEdge : uint32
  {
    Left = ToUnderlying(Edge::Left),
    Top = ToUnderlying(Edge::Top),
    Right = ToUnderlying(Edge::Right),
    Bottom = ToUnderlying(Edge::Bottom),
  };
}