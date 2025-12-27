#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Commands/CommandType.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Enums/BufferBitFlags.hpp"
#include "Krystal.Gfx/Enums/FilterMode.hpp"
#include "Krystal.Lib/String/StringRef.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Macros.hpp"
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

  struct ClearRenderTarget
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("ClearRenderTarget");

    BufferBitFlags Clear {BufferBitFlags::All};
    Gfx::ColourbPremultiplied Colour {Gfx::Colours::Black};
    float Depth {1.f};
    uint32 Stencil {0u};
  };

  struct BlitRenderTarget
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("BlitRenderTarget");

    RenderTargetHandle Source;
    Maths::Vec2 SourcePosition;
    Maths::Vec2 SourceSize;

    RenderTargetHandle Destination;
    Maths::Vec2 DestinationPosition;
    Maths::Vec2 DestinationSize;

    FilterMode Filter {FilterMode::Linear};
    BufferBitFlags Mask {BufferBitFlags::Colour};
  };

  struct CompositeRenderTargetWithOpacity
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("CompositeRenderTargetWithOpacity");
    RenderTargetHandle Source;
    RenderTargetHandle Destination;
    float Opacity {1.f};
  };

  struct CompositeRenderTarget
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("CompositeRenderTarget");
    RenderTargetHandle Source;
    RenderTargetHandle Destination;
  };

  struct DrawShape2D
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("DrawShape2D");

    MeshHandle Mesh;
    Gfx::TextureHandle Texture;
    Maths::Mat4 Transform;
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
}