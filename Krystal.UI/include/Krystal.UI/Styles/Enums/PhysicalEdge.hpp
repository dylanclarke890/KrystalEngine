#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Styles/Enums/Edge.hpp"
#include <utility>

namespace Krys::UI
{
  enum class PhysicalEdge : uint32
  {
    Left = std::to_underlying(Edge::Left),
    Top = std::to_underlying(Edge::Top),
    Right = std::to_underlying(Edge::Right),
    Bottom = std::to_underlying(Edge::Bottom),
  };
}