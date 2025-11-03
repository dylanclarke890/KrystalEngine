#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Commands/CommandType.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  namespace Commands
  {
    struct DrawRect
    {
      constexpr static CommandType Type = KRYS_CMD_TYPE("DrawRect");

      Gfx::Colour BackgroundColour;
      Gfx::Colour BorderColour;
      Maths::Vec2 Position;
      Maths::Vec2 Size;
      float BorderThickness {0.f};
      float BorderRadius {0.f};
    };

    struct BindRenderTarget
    {
      constexpr static CommandType Type = KRYS_CMD_TYPE("BindRenderTarget");
      RenderTargetHandle RenderTarget;
    };

    struct DrawRenderTargetColourAttachment
    {
      constexpr static CommandType Type = KRYS_CMD_TYPE("DrawRenderTargetColourAttachment");

      RenderTargetHandle Source;
      uint32 ColourAttachmentIndex {0u};
      Maths::Vec2 Position;
      Maths::Vec2 Size;
      float Opacity {1.f};
    };
  }

  namespace CommandTypes
  {
    constexpr CommandType BindRenderTarget = Commands::BindRenderTarget::Type;
    constexpr CommandType DrawRect = Commands::DrawRect::Type;
    constexpr CommandType DrawRenderTargetColourAttachment = Commands::DrawRenderTargetColourAttachment::Type;
  }
}