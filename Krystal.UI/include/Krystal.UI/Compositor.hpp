#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
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

    struct State
    {
      ElementHandle CurrentElement;
      Maths::Vec2 ParentOffset;
    };

    struct LayerContext
    {
      Gfx::RenderTargetHandle Target;
      size_t CommandListIndex {0u};
    };

    struct CachedLayer
    {
      Gfx::RenderTargetHandle Target;
      Maths::Vec2 Size;
      uint64 Hash;
    };

  private:
    Gfx::IContext &_context;
    Gfx::IRenderer &_renderer;
    List<Gfx::CommandList> _commandLists;
    Map<ElementHandle, CachedLayer> _cachedLayers;
    Stack<LayerContext> _layerStack;
    Set<ElementHandle> _usedLayers;
    List<Gfx::RenderTargetHandle> _pendingDestruction;

  public:
    Compositor(Gfx::IContext &context, Gfx::IRenderer &renderer) noexcept
        : _context(context), _renderer(renderer)
    {
    }

    ~Compositor() = default;

    void Render(Document &document, Gfx::RenderTargetHandle renderTarget = {}) noexcept
    {
      if (!renderTarget.IsValid())
      {
        renderTarget = _context.RenderTargets().GetScreenRenderTarget();
      }

      auto targetDimensions = _context.RenderTargets().GetDimensions(renderTarget);
      document.Reflow(targetDimensions.x, targetDimensions.y);

      PushLayer(document.Body(), renderTarget);
      BindRenderTargetCommand(renderTarget);
      RenderElement(document, document.Body(), {0.f, 0.f});
      PopLayer();

      // Submit child command lists first, since parents composite from their results.
      for (auto it = _commandLists.rbegin(); it != _commandLists.rend(); ++it)
      {
        _renderer.Submit(*it);
      }
      _commandLists.clear();

      for (auto it = _cachedLayers.begin(); it != _cachedLayers.end();)
      {
        if (_usedLayers.find(it->first) == _usedLayers.end())
        {
          _pendingDestruction.push_back(it->second.Target);
          it = _cachedLayers.erase(it);
        }
        else
        {
          ++it;
        }
      }
      _usedLayers.clear();

      for (auto &handle : _pendingDestruction)
      {
        _context.RenderTargets().Destroy(handle);
      }
      _pendingDestruction.clear();
    }

  private:
    Gfx::CommandList &CurrentCommandList() noexcept
    {
      assert(!_layerStack.empty() && "No active layer");
      return _commandLists.at(_layerStack.top().CommandListIndex);
    }

    void PushLayer(ElementHandle owner, Gfx::RenderTargetHandle renderTarget) noexcept
    {
      _usedLayers.insert(owner);
      _commandLists.emplace_back();
      _layerStack.push({renderTarget, _commandLists.size() - 1u});
    }

    void PopLayer() noexcept
    {
      _layerStack.pop();
    }

    void RenderElement(Document &document, ElementHandle handle, const Maths::Vec2 &parentOffset)
    {
      using namespace Maths;
      using namespace Gfx;

      auto &element = document.Get(handle);
      auto node = element.LayoutNode;

      Vec2 relativePosition = {NodeLayoutGetLeft(node), NodeLayoutGetTop(node)};
      Vec2 absolutePosition = parentOffset + relativePosition;
      Vec2 size = {NodeLayoutGetWidth(node), NodeLayoutGetHeight(node)};

      if (!document.ElementRequiresLayer(handle))
      {
        RenderElementContents(node, absolutePosition, size, element, document);
        return;
      }

      uint64 hash = ComputeLayerHash(element);
      // TODO: we need to handle when child elements change and invalidate the cache
      if (auto existing = _cachedLayers.find(handle); existing != _cachedLayers.end())
      {
        auto &cachedLayer = existing->second;
        if (cachedLayer.Target.IsValid() && cachedLayer.Size == size && cachedLayer.Hash == hash)
        {
          float opacity = document.ElementStyleGetOpacity(handle);
          DrawRenderTargetColourAttachmentCommand(cachedLayer.Target, absolutePosition, size, opacity);
          return;
        }
        else
        {
          _pendingDestruction.push_back(cachedLayer.Target);
          _cachedLayers.erase(existing);
        }
      }

      auto layerRenderTarget = _context.RenderTargets().Create({
        .Width = static_cast<uint32>(size.x),
        .Height = static_cast<uint32>(size.y),
        .Attachments = {{AttachmentType::Colour, PixelFormat::R8G8B8A8}},
      });

      PushLayer(handle, layerRenderTarget);
      _cachedLayers[handle] = {.Target = layerRenderTarget, .Size = size, .Hash = hash};
      BindRenderTargetCommand(layerRenderTarget);
      RenderElementContents(node, relativePosition, size, element, document);
      PopLayer();

      float opacity = document.ElementStyleGetOpacity(handle);
      DrawRenderTargetColourAttachmentCommand(layerRenderTarget, absolutePosition, size, opacity);
    }

    void RenderElementContents(NodeRef node, const Maths::Vec2 &position, const Maths::Vec2 &size,
                               Element &element, Document &document)
    {
      CurrentCommandList().Push(Gfx::Commands::DrawRect {
        .BackgroundColour = NodeStyleGetBackgroundColour(node),
        .BorderColour = NodeStyleGetBorderColour(node),
        .Position = position,
        .Size = size,
      });

      if (element.TextContent.Text.IsValid())
      {
        Gfx::FontFamilyHandle family = NodeStyleGetFontFamily(element.LayoutNode);
        if (!family.IsValid())
        {
          family = _context.Fonts().GetDefaultFontFamily();
        }

        float textLeft = NodeLayoutGetLeft(element.TextContent.LayoutNode);
        float textTop = NodeLayoutGetTop(element.TextContent.LayoutNode);

        CurrentCommandList().Push(Gfx::Commands::DrawText {
          .Text = element.TextContent.Text,
          .Position = position + Maths::Vec2 {textLeft, textTop},
          .FontFamily = family,
          .FontSize = NodeStyleGetFontSize(element.LayoutNode),
          .Colour = NodeStyleGetTextColour(element.LayoutNode),
        });
      }

      for (auto &child : element.Children)
      {
        RenderElement(document, child, position);
      }
    }

    void BindRenderTargetCommand(Gfx::RenderTargetHandle target)
    {
      CurrentCommandList().Push(Gfx::Commands::BindRenderTarget {
        .RenderTarget = target,
      });
    }

    void DrawRenderTargetColourAttachmentCommand(Gfx::RenderTargetHandle source, const Maths::Vec2 &position,
                                                 const Maths::Vec2 &size, float opacity)
    {
      CurrentCommandList().Push(Gfx::Commands::DrawRenderTargetColourAttachment {
        .Source = source,
        .ColourAttachmentIndex = 0u,
        .Position = position,
        .Size = size,
        .Opacity = opacity,
      });
    }

    uint64 ComputeLayerHash(const Element &e)
    {
      auto node = e.LayoutNode;
      uint64 h = HashUtils::HashCombine(NodeLayoutGetLeft(node), NodeLayoutGetTop(node));
      h = HashUtils::HashCombine(h, NodeLayoutGetWidth(node), NodeLayoutGetHeight(node));
      h = HashUtils::HashCombine(h, NodeStyleGetBackgroundColour(node), NodeStyleGetBorderColour(node));
      h = HashUtils::HashCombine(h, NodeStyleGetTextColour(node));
      if (e.TextContent.Text.IsValid())
      {
        h = HashUtils::HashCombine(h, _context.Strings().Get(e.TextContent.Text));
      }

      return h;
    }
  };
}