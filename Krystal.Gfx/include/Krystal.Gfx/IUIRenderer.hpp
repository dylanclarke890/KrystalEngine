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
    Colour Colour = Colours::Transparent;

    constexpr static VertexBufferLayout Layout()
    {
      return {
        {VertexAttributeType::Float, 2}, // position
        {VertexAttributeType::Float, 2}, // texcoord
        {VertexAttributeType::Float, 4}, // colour
      };
    }
  };

  struct UIElementUboData
  {
    Maths::Vec2 RectSize;
    Maths::Vec2 BorderThicknessRadius; // x: border thickness, y: border radius
    Colour FillColour;
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