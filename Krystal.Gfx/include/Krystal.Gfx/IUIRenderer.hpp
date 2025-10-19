#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/VertexBufferLayout.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Document.hpp"

namespace Krys::Gfx
{
  struct UIElementVertex
  {
    Maths::Vec2 Position;
    Maths::Vec2 UV;

    constexpr static VertexBufferLayout Layout()
    {
      return {
        {VertexAttributeType::Float, 2}, // position
        {VertexAttributeType::Float, 2}, // texcoord
      };
    }
  };

  struct UIElementInstanceData
  {
    Colour BackgroundColour;
    Colour BorderColour;
    Maths::Vec4 PositionAndSize;       // xy: position, zw: size
    Maths::Vec2 BorderThicknessRadius; // x: border thickness, y: border radius

    constexpr static VertexBufferLayout Layout()
    {
      return {
        {VertexAttributeType::Float, 4, VertexInputRate::PerInstance}, // background colour
        {VertexAttributeType::Float, 4, VertexInputRate::PerInstance}, // border colour
        {VertexAttributeType::Float, 4, VertexInputRate::PerInstance}, // quad position and size
        {VertexAttributeType::Float, 2, VertexInputRate::PerInstance}, // border thickness and radius
      };
    }
  };

  class IUIRenderer
  {
  public:
    IUIRenderer() noexcept = default;
    virtual ~IUIRenderer() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    virtual void BeginFrame() = 0;

    virtual void EndFrame() = 0;

    virtual void Render(UI::Document &doc, float width, float height) = 0;
  };

  Expected<Unique<IUIRenderer>> CreateUIRenderer(IContext &context) noexcept;
}