#pragma once

#include "Krystal.Core/Commands/CommandType.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Enums/BufferBitFlags.hpp"
#include "Krystal.Gfx/Enums/FilterMode.hpp"
#include "Krystal.Gfx/Handle.hpp"

namespace krys::Gfx::Commands
{
  struct SetScissor
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("SetScissor");

    Vec2 Position;
    Vec2 Size;
  };

  struct ClearScissor
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("ClearScissor");
  };

  struct SetViewport
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("SetViewport");

    Vec2 Position;
    Vec2 Size;
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
    Vec2 SourcePosition;
    Vec2 SourceSize;

    RenderTargetHandle Destination;
    Vec2 DestinationPosition;
    Vec2 DestinationSize;

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
    Mat4 Transform;
    uint32 InstanceCount {1u};
  };

  struct DrawText
  {
    constexpr static CommandType Type = KRYS_CMD_TYPE("DrawText");

    utf8_stringview Text;
    Vec2 Position;
    FontFamilyHandle FontFamily;
    float FontSize {16.f};
    Gfx::ColourbPremultiplied Colour {Gfx::Colours::Black};
  };
}