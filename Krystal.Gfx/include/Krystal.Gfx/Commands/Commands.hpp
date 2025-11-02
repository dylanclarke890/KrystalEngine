#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Commands/CommandType.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  struct BindRenderTargetCommand
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("BindRenderTargetCommand");
    RenderTargetHandle RenderTarget;
  };

  struct RectCommand
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("RectCommand");

    Gfx::Colour BackgroundColour;
    Gfx::Colour BorderColour;
    Maths::Vec2 Position;
    Maths::Vec2 Size;
    float BorderThickness {0.f};
    float BorderRadius {0.f};
  };

  namespace Commands
  {
    constexpr CommandType BindRenderTarget = BindRenderTargetCommand::Type;
    constexpr CommandType Rect = RectCommand::Type;
  }
}