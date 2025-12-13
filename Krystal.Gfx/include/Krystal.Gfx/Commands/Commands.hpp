#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Commands/CommandType.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/String/StringRef.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx::Commands
{
  struct SetScissor
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("SetScissor");
    Maths::Vec2 Position;
    Maths::Vec2 Size;
  };

  struct ClearScissor
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("ClearScissor");
  };

  struct SetViewport
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("SetViewport");
    Maths::Vec2 Position;
    Maths::Vec2 Size;
  };

  struct BindRenderTarget
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("BindRenderTarget");
    RenderTargetHandle RenderTarget;
  };

  struct DrawShape2D
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("DrawShape2D");

    MeshHandle Mesh;
    Gfx::TextureHandle Texture;
    Maths::Mat4 Transform;
    Maths::Vec2 Translation;
    uint32 InstanceCount {1u};
  };

  struct DrawText
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("DrawText");

    StringRef Text;
    Maths::Vec2 Position;
    FontFamilyHandle FontFamily;
    float FontSize {16.f};
    Gfx::ColourbPremultiplied Colour {Gfx::Colours::Black};
  };

  struct DrawRect
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("DrawRect");

    Gfx::ColourbPremultiplied BackgroundColour;
    Gfx::ColourbPremultiplied BorderColourLeft;
    Gfx::ColourbPremultiplied BorderColourRight;
    Gfx::ColourbPremultiplied BorderColourTop;
    Gfx::ColourbPremultiplied BorderColourBottom;
    Maths::Vec2 Position;
    Maths::Vec2 Size;
    float BorderWidth {0.f};
    float BorderRadius {0.f};
  };
}