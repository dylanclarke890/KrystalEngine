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
#include "Krystal.UI/RenderBox.hpp"
#include <cassert>

namespace Krys::UI
{
  class Compositor
  {
    NO_COPY_MOVE(Compositor)

    struct RenderContext
    {
      Maths::Vec2 Origin;
    };

    constexpr static Maths::Vec2 OriginZero = {0.f, 0.f};

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
      RenderElement(document, document.Body(), {OriginZero});

      // Submit command list
      _renderer.Submit(_commands);
    }

  private:
    void RenderElement(Document &document, ElementHandle handle, const RenderContext &ctx)
    {
      using namespace Maths;

      auto &element = document.Get(handle);
      NodeRef node = element.LayoutNode;

      Vec2 position = ctx.Origin + Vec2 {NodeLayoutGetLeft(node), NodeLayoutGetTop(node)};
      Vec2 size = Vec2 {NodeLayoutGetWidth(node), NodeLayoutGetHeight(node)};

      RenderContext childCtx {position};
      RenderBox box = BuildRenderBox(node);

      PaintElement(element, node, childCtx, box);

      for (ElementHandle child : element.Children)
      {
        RenderElement(document, child, childCtx);
      }
    }

    void PaintElement(Element &element, NodeRef node, const RenderContext &ctx, const RenderBox &renderBox)
    {
      using namespace Gfx;
      using namespace Maths;

      if (element.Geometries.empty())
      {
        MeshData data;

        auto &bwidths = renderBox.GetBorderWidths();
        auto &bcolours = NodeStyleGetBorderColours(node);

        MeshDataUtils::GenerateQuad(data, renderBox.GetFillOffset(), renderBox.GetFillSize(),
                                    NodeStyleGetBackgroundColour(node));

        if (bwidths[TopEdge] > 0.f && bcolours[TopEdge].alpha > 0.f)
        {
          Vec2 size = {renderBox.GetFillSize().x, bwidths[TopEdge]};
          MeshDataUtils::GenerateQuad(data, {bwidths[LeftEdge], 0.f}, size, bcolours[TopEdge]);
        }

        if (bwidths[LeftEdge] > 0.f && bcolours[LeftEdge].alpha > 0.f)
        {
          Vec2 size = {bwidths[LeftEdge], renderBox.GetFillSize().y + bwidths[TopEdge] + bwidths[BottomEdge]};
          MeshDataUtils::GenerateQuad(data, {0.f, 0.f}, size, bcolours[LeftEdge]);
        }

        if (bwidths[BottomEdge] > 0.f && bcolours[BottomEdge].alpha > 0.f)
        {
          Vec2 size = {renderBox.GetFillSize().x, bwidths[BottomEdge]};
          MeshDataUtils::GenerateQuad(data, {bwidths[LeftEdge], renderBox.GetFillSize().y + bwidths[TopEdge]},
                                      size, bcolours[BottomEdge]);
        }

        if (bwidths[RightEdge] > 0.f && bcolours[RightEdge].alpha > 0.f)
        {
          Vec2 size = {bwidths[RightEdge],
                       renderBox.GetFillSize().y + bwidths[TopEdge] + bwidths[BottomEdge]};
          MeshDataUtils::GenerateQuad(data, {renderBox.GetFillSize().x + bwidths[LeftEdge], 0.f}, size,
                                      bcolours[RightEdge]);
        }

        MeshDesc desc {.Vertices = data.Vertices,
                       .Indices = data.Indices,
                       .Layout = Vertex::Position2D_ColourbPremultiplied_UV::Layout(),
                       .Primitive = PrimitiveType::Triangles,
                       .Type = MeshType::Static};

        element.Geometries.push_back({.Mesh = _context.Meshes().Create(desc), .Translation = {}});
      }

      // Emit draw commands for geometry
      for (auto &geometry : element.Geometries)
      {
        Mat4 transform = Maths::Translate(Vec3 {ctx.Origin + geometry.Translation, 0.f});
        _commands.Push(Commands::DrawShape2D {
          .Mesh = geometry.Mesh,
          .Texture = {},
          .Transform = transform,
          .InstanceCount = 1,
        });
      }

      // Emit text if present
      if (element.TextContent.Text.IsValid())
      {
        Vec2 textPosition = ctx.Origin
                            + Vec2 {NodeLayoutGetLeft(element.TextContent.LayoutNode),
                                    NodeLayoutGetTop(element.TextContent.LayoutNode)};

        auto fontFamily = NodeStyleGetFontFamily(node);
        if (!fontFamily.IsValid())
        {
          fontFamily = _context.Fonts().GetDefaultFontFamily();
        }

        _commands.Push(Commands::DrawText {.Text = element.TextContent.Text,
                                           .Position = textPosition,
                                           .FontFamily = fontFamily,
                                           .FontSize = NodeStyleGetFontSize(node),
                                           .Colour = NodeStyleGetTextColour(node)});
      }
    }

    RenderBox BuildRenderBox(NodeRef node)
    {
      using namespace Maths;

      float width = NodeLayoutGetWidth(node);
      float height = NodeLayoutGetHeight(node);
      Vec2 borderBoxSize = {width, height};

      EdgeSizes borderWidths = {NodeLayoutGetBorder(node, Edge::Top), NodeLayoutGetBorder(node, Edge::Right),
                                NodeLayoutGetBorder(node, Edge::Bottom),
                                NodeLayoutGetBorder(node, Edge::Left)};

      CornerSizes borderRadii = NodeStyleGetBorderRadii(node);

      Vec2 fillSize = {borderBoxSize.x - (borderWidths[LeftEdge] + borderWidths[RightEdge]),
                       borderBoxSize.y - (borderWidths[TopEdge] + borderWidths[BottomEdge])};
      fillSize.x = std::max(0.f, fillSize.x);
      fillSize.y = std::max(0.f, fillSize.y);

      Vec2 borderOffset = {NodeLayoutGetPadding(node, Edge::Left), NodeLayoutGetPadding(node, Edge::Top)};

      return RenderBox(fillSize, borderOffset, borderWidths, borderRadii);
    }
  };
}