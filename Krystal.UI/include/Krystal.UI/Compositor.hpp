#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.Gfx/Utils/MeshDataUtils.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/HashUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/Types.hpp"
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

  public:
    explicit LayerPool(Gfx::IContext &context) : _context(context), _entries()
    {
    }

    void BeginFrame()
    {
      for (auto &e : _entries)
      {
        e.InUse = false;
      }
    }

    Gfx::RenderTargetHandle Acquire(Maths::Vec2 size)
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
        .Width = (uint32)size.x,
        .Height = (uint32)size.y,
        .Samples = 2u,
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

    void Clear()
    {
      for (auto &e : _entries)
      {
        _context.RenderTargets().Destroy(e.Target);
      }
      _entries.clear();
    }
  };

  class Compositor
  {
    NO_COPY_MOVE(Compositor)

    struct RenderContext
    {
      Maths::Vec2 Origin;
    };

    struct PostProcessTargets
    {
      /// @brief The primary post-process render target. Resolve MSAA into this target.
      Gfx::RenderTargetHandle Primary;
      /// @brief The secondary post-process render target. Used for ping-ponging effects.
      Gfx::RenderTargetHandle Secondary;
    };

  private:
    Gfx::IContext &_context;
    Gfx::IRenderer &_renderer;
    LayerPool _layerPool;
    Gfx::CommandList _commands;
    Stack<Layer> _layerStack;
    Maths::Vec2 _viewportSize;
    PostProcessTargets _post;

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

      Maths::Vec2 dimensions = _context.RenderTargets().GetDimensions(target);
      if (_viewportSize != dimensions)
      {
        _viewportSize = dimensions;
        _layerPool.Clear();
        LayoutEngine::Reflow(document.Get(document.Body()), _viewportSize);
        RebuildPostProcessTargets();
      }

      _commands.Clear();
      _layerStack.push({.Target = target});

      // We don't clear the root target here as we may be compositing to the screen
      BindCurrentLayerRenderTarget(false);
      _layerPool.BeginFrame();

      RenderElement(document, document.Body(), {{0.f, 0.f}});
      _layerStack.pop();

      _renderer.Submit(_commands);
    }

  private:
    void RebuildPostProcessTargets()
    {
      using namespace Gfx;

      if (_post.Primary.IsValid())
      {
        _context.RenderTargets().Destroy(_post.Primary);
      }
      if (_post.Secondary.IsValid())
      {
        _context.RenderTargets().Destroy(_post.Secondary);
      }

      const RenderTargetDesc desc {
        .Width = static_cast<uint32>(_viewportSize.x),
        .Height = static_cast<uint32>(_viewportSize.y),
        .Samples = 1u,
        .Attachments = {{
          .Type = AttachmentType::Colour,
          .Format = PixelFormat::R8G8B8A8,
          .OnLoad = AttachmentLoadOp::Clear,
          .OnStore = AttachmentStoreOp::Store,
          .ClearValue = AttachmentClearValue::Colour(Colours::Transparent.ToVec4()),
        }},
      };

      _post.Primary = _context.RenderTargets().Create(desc);
      _post.Secondary = _context.RenderTargets().Create(desc);
    }

    /// @brief Pushes a command to bind the current layer's render target and optionally clears it.
    void BindCurrentLayerRenderTarget(bool clear)
    {
      using namespace Gfx;
      using namespace Gfx::Commands;

      _commands.Push(BindRenderTarget {.RenderTarget = GetCurrentRenderTarget()});
      if (clear)
      {
        _commands.Push(ClearRenderTarget {.Clear = BufferBitFlags::Colour, .Colour = Colours::Transparent});
      }
    }

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

      RenderTargetHandle renderTarget = _layerPool.Acquire(_viewportSize);
      Layer layer {.Target = renderTarget, .Opacity = NodeStyleGetOpacity(element.LayoutNode)};

      _layerStack.push(layer);
      // We clear the layer as it's a new render target
      BindCurrentLayerRenderTarget(true);

      return layer;
    }

    void EndLayer(const Layer &layer, const RenderContext &ctx)
    {
      using namespace Gfx;
      using namespace Maths;

      _layerStack.pop();
      // We don't clear here as we're in the process of compositing
      BindCurrentLayerRenderTarget(false);

      _commands.Push(Commands::ComposeRenderTargets {
        .Source = layer.Target,
        .Destination = GetCurrentRenderTarget(),
        .Opacity = layer.Opacity,
      });
    }
  };
}