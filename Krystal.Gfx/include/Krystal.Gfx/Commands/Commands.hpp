#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.Gfx/Commands/CommandType.hpp"
#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  struct RectCommand
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("RectCommand");
    
    Gfx::Colour BackgroundColour;
    Gfx::Colour BorderColour;
    Maths::Vec2 Position;
    Maths::Vec2 Size;
    float BorderThickness;
    float BorderRadius;
  };
}