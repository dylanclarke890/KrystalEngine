#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/HashUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.UI/Document.hpp"

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
      // TODO: use index into _commandLists instead of holding reference? Dangles when vector resizes.
      Gfx::CommandList &Commands;
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
    Stack<LayerContext> _layerStack;
    List<Gfx::RenderTargetHandle> _usedTempRenderTargets;
    uint32 _lastDrawnElementCount {0u};
    uint32 _drawnElementCount {0u};
    Map<ElementHandle, CachedLayer> _cachedLayers;

  public:
    Compositor(Gfx::IContext &context, Gfx::IRenderer &renderer) noexcept
        : _context(context), _renderer(renderer)
    {
    }

    ~Compositor() = default;

    void Render(Document &document, Gfx::RenderTargetHandle renderTarget = {}) noexcept
    {
      _drawnElementCount = 0u;
      if (!renderTarget.IsValid())
      {
        renderTarget = _context.RenderTargets().GetScreenRenderTarget();
      }

      auto targetDimensions = _context.RenderTargets().GetDimensions(renderTarget);
      document.Reflow(targetDimensions.x, targetDimensions.y);

      _layerStack = {};
      _commandLists.clear();
      // TODO: we do this to avoid reallocations during rendering (it breaks things), but we should have a better strategy.
      _commandLists.reserve(16u);

      _commandLists.emplace_back();
      _layerStack.push({renderTarget, _commandLists.back()});

      _layerStack.top().Commands.Push(Gfx::Commands::BindRenderTarget {renderTarget});
      RenderElement(document, {document.Body(), {0.f, 0.f}});

      // Submit child command lists first, since parents composite from their results.
      for (auto it = _commandLists.rbegin(); it != _commandLists.rend(); ++it)
      {
        _renderer.Submit(*it);
      }

      for (Gfx::RenderTargetHandle rt : _usedTempRenderTargets)
      {
        _context.RenderTargets().Release(rt);
      }
      _usedTempRenderTargets.clear();

      if (_lastDrawnElementCount != _drawnElementCount)
      {
        KRYS_DEBUG("Compositor rendered {} elements (previously {})", _drawnElementCount,
                   _lastDrawnElementCount);
        _lastDrawnElementCount = _drawnElementCount;
      }
    }

  private:
    void RenderElement(Document &document, const State &state)
    {
      _drawnElementCount++;
      auto &element = document.Get(state.CurrentElement);
      auto node = element.LayoutNode;

      // if (!NodeGetHasNewLayout(node))
      //{
      //   return;
      // }

      // NodeSetHasNewLayout(node, false);
      Maths::Vec2 position = {NodeLayoutGetLeft(node), NodeLayoutGetTop(node)};
      float w = NodeLayoutGetWidth(node);
      float h = NodeLayoutGetHeight(node);

      if (document.ElementRequiresLayer(state.CurrentElement))
      {
        //if (auto existing = _cachedLayers.find(state.CurrentElement); existing != _cachedLayers.end())
        //{
        //  uint64 hash = ComputeLayerHash(element);
        //  auto &cachedLayer = existing->second;
        //  bool isValid = cachedLayer.Target.IsValid() && cachedLayer.Size == Maths::Vec2 {w, h}
        //                 && cachedLayer.Hash == hash;

        //  if (isValid)
        //  {
        //    float opacity = document.ElementStyleGetOpacity(state.CurrentElement);
        //    DrawRenderTargetColourAttachmentCommand(cachedLayer.Target, state.ParentOffset + position, {w, h},
        //                                            opacity);
        //    return;
        //  }
        //}

        Gfx::RenderTargetDesc desc {
          .Width = static_cast<uint32>(w),
          .Height = static_cast<uint32>(h),
          .Attachments = {{Gfx::AttachmentType::Colour, Gfx::PixelFormat::R8G8B8A8}},
        };
        auto layerRenderTarget = _context.RenderTargets().Acquire(desc);
        _usedTempRenderTargets.push_back(layerRenderTarget);

        _commandLists.emplace_back();
        _layerStack.push({layerRenderTarget, _commandLists.back()});

        Gfx::Commands::BindRenderTarget bindRTCmd;
        bindRTCmd.RenderTarget = layerRenderTarget;
        _layerStack.top().Commands.Push(bindRTCmd);

        Gfx::Commands::DrawRect drawRectCmd;
        drawRectCmd.Position = position;
        drawRectCmd.Size = {w, h};
        drawRectCmd.BackgroundColour = NodeStyleGetBackgroundColour(node);
        drawRectCmd.BorderColour = NodeStyleGetBorderColour(node);
        _layerStack.top().Commands.Push(drawRectCmd);

        if (element.TextContent.Text.IsValid())
        {
          DrawTextCommand(element, position);
        }

        for (auto &child : element.Children)
        {
          RenderElement(document, {child, position});
        }

        auto &layerContext = _layerStack.top();
        _layerStack.pop();

        float opacity = document.ElementStyleGetOpacity(state.CurrentElement);
        DrawRenderTargetColourAttachmentCommand(layerRenderTarget, state.ParentOffset + position, {w, h},
                                                opacity);
      }
      else
      {
        Gfx::Commands::DrawRect command;
        command.Position = state.ParentOffset + position;
        command.Size = {w, h};
        command.BackgroundColour = NodeStyleGetBackgroundColour(node);
        command.BorderColour = NodeStyleGetBorderColour(node);
        _layerStack.top().Commands.Push(command);

        if (element.TextContent.Text.IsValid())
        {
          DrawTextCommand(element, state.ParentOffset + position);
        }

        for (auto &child : element.Children)
        {
          RenderElement(document, {child, state.ParentOffset + position});
        }
      }
    }

    void DrawTextCommand(Element &element, const Maths::Vec2 &offset)
    {
      Gfx::FontFamilyHandle family = NodeStyleGetFontFamily(element.LayoutNode);
      if (!family.IsValid())
      {
        family = _context.Fonts().GetDefaultFontFamily();
      }

      float left = NodeLayoutGetLeft(element.TextContent.LayoutNode);
      float top = NodeLayoutGetTop(element.TextContent.LayoutNode);

      _layerStack.top().Commands.Push(Gfx::Commands::DrawText {
        .Text = element.TextContent.Text,
        .Position = offset + Maths::Vec2(left, top),
        .FontFamily = family,
        .FontSize = NodeStyleGetFontSize(element.LayoutNode),
        .Colour = NodeStyleGetTextColour(element.LayoutNode),
      });
    }

    void DrawRenderTargetColourAttachmentCommand(Gfx::RenderTargetHandle source, const Maths::Vec2 &position,
                                                 const Maths::Vec2 &size, float opacity)
    {
      _layerStack.top().Commands.Push(Gfx::Commands::DrawRenderTargetColourAttachment {
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