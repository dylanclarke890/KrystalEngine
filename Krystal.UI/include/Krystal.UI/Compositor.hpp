#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.Gfx/Utils/MeshDataUtils.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/HashUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Set.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.UI/Document.hpp"
#include <cassert>

namespace Krys::UI
{
  class Compositor
  {
    NO_COPY_MOVE(Compositor)

  private:
    Gfx::IContext &_context;
    Gfx::IRenderer &_renderer;
    Gfx::CommandList _commands;
    Gfx::MeshHandle _quadMesh;

  public:
    Compositor(Gfx::IContext &context, Gfx::IRenderer &renderer) : _context(context), _renderer(renderer)
    {
      using namespace Gfx;

      MeshData data;
      MeshDataUtils::GenerateQuad(data, {0.f, 0.f}, {1.f, 1.f}, Colours::White);

      _quadMesh = _context.Meshes().Create({
        .Vertices = data.Vertices,
        .Indices = data.Indices,
        .Layout = Vertex::Position2D_ColourbPremultiplied_UV::Layout(),
        .Primitive = PrimitiveType::Triangles,
        .Type = MeshType::Static,
      });
    }

    void Render(Document &document, Gfx::RenderTargetHandle target = {})
    {
      using namespace Gfx;

      if (!target.IsValid())
      {
        target = _context.RenderTargets().GetScreenRenderTarget();
      }

      _commands = CommandList {};
      _commands.Push(Commands::BindRenderTarget {.RenderTarget = target});

      // Layout pass
      auto dimensions = _context.RenderTargets().GetDimensions(target);
      LayoutEngine::Reflow(document.Get(document.Body()), dimensions);

      // Recursive traversal from root
      RenderElement(document, document.Body(), {0.f, 0.f});

      // Submit command list
      _renderer.Submit(_commands);
    }

  private:
    void RenderElement(Document &document, ElementHandle handle, const Maths::Vec2 &origin)
    {
      using namespace Maths;

      auto &element = document.Get(handle);
      NodeRef node = element.LayoutNode;

      Vec2 position = origin + Vec2 {NodeLayoutGetLeft(node), NodeLayoutGetTop(node)};
      Vec2 size = Vec2 {NodeLayoutGetWidth(node), NodeLayoutGetHeight(node)};

      PaintElement(element, node, position, size);

      for (ElementHandle child : element.Children)
      {
        RenderElement(document, child, position);
      }
    }

    void PaintElement(Element &elem, NodeRef node, const Maths::Vec2 &absPos, const Maths::Vec2 &size)
    {
      using namespace Gfx;
      using namespace Maths;

      // Build background geometry on-demand
      if (elem.Geometries.empty())
      {
        MeshData data;
        MeshDataUtils::GenerateQuad(data, {0.f, 0.f}, size, NodeStyleGetBackgroundColour(node));

        MeshDesc desc {.Vertices = data.Vertices,
                       .Indices = data.Indices,
                       .Layout = Vertex::Position2D_ColourbPremultiplied_UV::Layout(),
                       .Primitive = PrimitiveType::Triangles,
                       .Type = MeshType::Static};

        elem.Geometries.push_back({.Mesh = _context.Meshes().Create(desc), .Translation = {}});
      }

      // Emit draw commands for geometry
      for (auto &geometry : elem.Geometries)
      {
        Mat4 transform = Maths::Translate(Vec3 {absPos + geometry.Translation, 0.f});
        _commands.Push(Commands::DrawShape2D {
          .Mesh = geometry.Mesh,
          .Texture = {},
          .Transform = transform,
          .InstanceCount = 1,
        });
      }

      // Emit text if present
      if (elem.TextContent.Text.IsValid())
      {
        Vec2 textPosition = absPos
                            + Vec2 {NodeLayoutGetLeft(elem.TextContent.LayoutNode),
                                    NodeLayoutGetTop(elem.TextContent.LayoutNode)};

        auto fontFamily = NodeStyleGetFontFamily(node);
        if (!fontFamily.IsValid())
        {
          fontFamily = _context.Fonts().GetDefaultFontFamily();
        }

        _commands.Push(Commands::DrawText {.Text = elem.TextContent.Text,
                                           .Position = textPosition,
                                           .FontFamily = fontFamily,
                                           .FontSize = NodeStyleGetFontSize(node),
                                           .Colour = NodeStyleGetTextColour(node)});
      }
    }
  };
}