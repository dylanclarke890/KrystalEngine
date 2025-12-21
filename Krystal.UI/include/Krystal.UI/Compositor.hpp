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
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.UI/Document.hpp"
#include "Krystal.UI/Geometry/GeometryUtils.hpp"
#include "Krystal.UI/Geometry/RenderBox.hpp"
#include <cassert>
#include <compare>

namespace Krys::UI
{
  struct Layer
  {
    Gfx::RenderTargetHandle Target;
    float Opacity {1.0f};
  };

  class LayerPool
  {
    struct Entry
    {
      Gfx::RenderTargetHandle Target;
      bool InUse = false;
    };

    Gfx::IContext &_context;
    List<Entry> _entries;
    Maths::Vec2 _size;

  public:
    explicit LayerPool(Gfx::IContext &context) : _context(context), _entries(), _size({0.f, 0.f})
    {
    }

    void BeginFrame(Maths::Vec2 size)
    {
      if (_size == size)
      {
        for (auto &e : _entries)
        {
          _context.RenderTargets().Destroy(e.Target);
        }
        _entries.clear();
      }

      _size = size;
      for (auto &e : _entries)
      {
        e.InUse = false;
      }
    }

    Gfx::RenderTargetHandle Acquire()
    {
      using namespace Gfx;

      for (auto &e : _entries)
      {
        if (!e.InUse)
        {
          e.InUse = true;
          return e.Target;
        }
      }

      // Allocate new full-size render target
      RenderTargetHandle target = _context.RenderTargets().Create({
        .Width = (uint32)_size.x,
        .Height = (uint32)_size.y,
        .Attachments = {{
          .Type = AttachmentType::Colour,
          .Format = PixelFormat::R8G8B8A8,
          .OnLoad = AttachmentLoadOp::Clear,
          .OnStore = AttachmentStoreOp::Store,
          .ClearValue = AttachmentClearValue::Colour(Colours::Transparent.ToVec4()),
        }},
      });

      _entries.push_back({target, true});
      return target;
    }
  };

  class Compositor
  {
    NO_COPY_MOVE(Compositor)

    struct RenderContext
    {
      Maths::Vec2 Origin;
    };

  private:
    Gfx::IContext &_context;
    Gfx::IRenderer &_renderer;
    LayerPool _layerPool;
    Gfx::CommandList _commands;
    Stack<Layer> _layerStack;

  public:
    Compositor(Gfx::IContext &context, Gfx::IRenderer &renderer)
        : _context(context), _renderer(renderer), _layerPool(context)
    {
    }

    void Render(Document &document, Gfx::RenderTargetHandle target = {})
    {
      using namespace Gfx;

      if (!target.IsValid())
      {
        target = _context.RenderTargets().GetScreenRenderTarget();
      }

      _commands = CommandList {};

      auto dimensions = _context.RenderTargets().GetDimensions(target);
      LayoutEngine::Reflow(document.Get(document.Body()), dimensions);
      _layerPool.BeginFrame(dimensions);

      _layerStack.push({.Target = target});
      _commands.Push(Commands::BindRenderTarget {.RenderTarget = target});

      RenderElement(document, document.Body(), {});
      _layerStack.pop();

      _renderer.Submit(_commands);
    }

  private:
    void RenderElement(Document &document, ElementHandle handle, const RenderContext &ctx)
    {
      using namespace Maths;

      auto &element = document.Get(handle);
      NodeRef node = element.LayoutNode;

      const bool createsLayer = CreatesLayer(element);

      Layer layer;
      if (createsLayer)
      {
        layer = BeginLayer(element);
      }

      Vec2 position = Vec2 {NodeLayoutGetLeft(node), NodeLayoutGetTop(node)};
      if (!createsLayer)
      {
        position += ctx.Origin;
      }

      RenderContext childCtx {position};
      PaintElement(element, childCtx);

      for (ElementHandle child : element.Children)
      {
        RenderElement(document, child, childCtx);
      }

      if (createsLayer)
      {
        EndLayer(layer, ctx);
      }
    }

    void PaintElement(Element &element, const RenderContext &ctx)
    {
      using namespace Gfx;
      using namespace Maths;

      NodeRef node = element.LayoutNode;
      if (element.Geometries.empty())
      {
        RenderBox renderBox = BuildRenderBox(node);

        MeshData data;

        GeometryUtils::GenerateBorderBackground(data, renderBox, NodeStyleGetBackgroundColour(node),
                                                NodeStyleGetBorderColours(node));

        MeshDesc desc {.Vertices = data.Vertices,
                       .Indices = data.Indices,
                       .Layout = Vertex::Layout(),
                       .Primitive = PrimitiveType::Triangles,
                       .Type = MeshType::Static};

        element.Geometries.push_back({.Mesh = _context.Meshes().Create(desc), .Translation = {}});
      }

      // Emit draw commands for geometry
      for (auto &geometry : element.Geometries)
      {
        Mat4 transform = Identity<Mat4>();
        transform = Maths::Translate(transform, Vec3 {ctx.Origin + geometry.Translation, 0.f});

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

      Vec2 fillSize = {borderBoxSize.x - (borderWidths[+BoxEdge::Left] + borderWidths[+BoxEdge::Right]),
                       borderBoxSize.y - (borderWidths[+BoxEdge::Top] + borderWidths[+BoxEdge::Bottom])};
      fillSize.x = std::max(0.f, fillSize.x);
      fillSize.y = std::max(0.f, fillSize.y);

      Vec2 borderOffset = {NodeLayoutGetPadding(node, Edge::Left), NodeLayoutGetPadding(node, Edge::Top)};

      return RenderBox(fillSize, borderOffset, borderWidths, borderRadii);
    }

    Gfx::RenderTargetHandle GetCurrentRenderTarget() const
    {
      assert(!_layerStack.empty() && "Layer stack is empty");
      return _layerStack.top().Target;
    }

    bool CreatesLayer(const Element &element)
    {
      return NodeStyleGetOpacity(element.LayoutNode) < 1.0f;
    }

    Layer BeginLayer(const Element &element)
    {
      using namespace Gfx;

      RenderTargetHandle renderTarget = _layerPool.Acquire();
      Layer layer {.Target = renderTarget, .Opacity = NodeStyleGetOpacity(element.LayoutNode)};

      _commands.Push(Commands::BindRenderTarget {.RenderTarget = renderTarget});
      _layerStack.push(layer);

      return layer;
    }

    void EndLayer(const Layer &layer, const RenderContext &ctx)
    {
      using namespace Gfx;
      using namespace Maths;

      _layerStack.pop();

      // Rebind parent target
      _commands.Push(Commands::BindRenderTarget {.RenderTarget = GetCurrentRenderTarget()});
      _commands.Push(Commands::ComposeRenderTargets {
        .Source = layer.Target,
        .Destination = GetCurrentRenderTarget(),
        .Opacity = layer.Opacity,
      });
    }
  };
}