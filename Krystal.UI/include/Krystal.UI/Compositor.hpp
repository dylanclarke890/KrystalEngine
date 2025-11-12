#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
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
      float ParentX;
      float ParentY;
    };

    struct LayerContext
    {
      Gfx::RenderTargetHandle Target;
      Gfx::CommandList &Commands;
    };

  private:
    Gfx::IContext &_context;
    Gfx::IRenderer &_renderer;
    List<Gfx::CommandList> _commandLists;
    Stack<LayerContext> _layerStack;
    List<Gfx::RenderTargetHandle> _usedTempRenderTargets;

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

      _layerStack = {};
      _commandLists.clear();
      _commandLists.reserve(16u);

      _commandLists.emplace_back();
      _layerStack.push({renderTarget, _commandLists.back()});

      _layerStack.top().Commands.Push(Gfx::Commands::BindRenderTarget {renderTarget});
      RenderElement(document, {document.Body(), 0.f, 0.f});

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
    }

  private:
    void RenderElement(Document &document, const State &state)
    {
      auto &element = document.Get(state.CurrentElement);
      auto node = element.LayoutNode;

      float x = NodeLayoutGetLeft(node);
      float y = NodeLayoutGetTop(node);
      float w = NodeLayoutGetWidth(node);
      float h = NodeLayoutGetHeight(node);

      if (document.ElementRequiresLayer(state.CurrentElement))
      {
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
        drawRectCmd.Position = {x, y};
        drawRectCmd.Size = {w, h};
        drawRectCmd.BackgroundColour = NodeStyleGetBackgroundColour(node);
        drawRectCmd.BorderColour = NodeStyleGetBorderColour(node);
        _layerStack.top().Commands.Push(drawRectCmd);

        if (element.TextContent.Text.IsValid())
        {
          DrawTextCommand(element, x, y);
        }

        for (auto &child : element.Children)
        {
          RenderElement(document, {child, x, y});
        }

        auto &layerContext = _layerStack.top();
        _layerStack.pop();

        Gfx::Commands::DrawRenderTargetColourAttachment drawRenderTargetCmd;
        drawRenderTargetCmd.Source = layerRenderTarget;
        drawRenderTargetCmd.ColourAttachmentIndex = 0u;
        drawRenderTargetCmd.Position = {state.ParentX + x, state.ParentY + y};
        drawRenderTargetCmd.Size = {w, h};
        drawRenderTargetCmd.Opacity = document.ElementStyleGetOpacity(state.CurrentElement);
        _layerStack.top().Commands.Push(drawRenderTargetCmd);
      }
      else
      {
        Gfx::Commands::DrawRect command;
        command.Position = {state.ParentX + x, state.ParentY + y};
        command.Size = {w, h};
        command.BackgroundColour = NodeStyleGetBackgroundColour(node);
        command.BorderColour = NodeStyleGetBorderColour(node);
        _layerStack.top().Commands.Push(command);

        if (element.TextContent.Text.IsValid())
        {
          DrawTextCommand(element, state.ParentX + x, state.ParentY + y);
        }

        for (auto &child : element.Children)
        {
          RenderElement(document, {child, state.ParentX + x, state.ParentY + y});
        }
      }
    }

    void DrawTextCommand(Element &element, float parentX, float parentY)
    {
      float posX = parentX + NodeLayoutGetLeft(element.TextContent.LayoutNode);
      float posY = parentY + NodeLayoutGetTop(element.TextContent.LayoutNode);

      Gfx::FontFamilyHandle family = NodeStyleGetFontFamily(element.LayoutNode);
      if (!family.IsValid())
      {
        family = _context.Fonts().GetDefaultFontFamily();
      }

      Gfx::Commands::DrawText drawTextCmd;
      drawTextCmd.Text = element.TextContent.Text;
      drawTextCmd.Position = {posX, posY};
      drawTextCmd.FontFamily = family;
      drawTextCmd.FontSize = NodeStyleGetFontSize(element.LayoutNode);
      drawTextCmd.Colour = NodeStyleGetTextColour(element.LayoutNode);
      _layerStack.top().Commands.Push(drawTextCmd);
    }
  };
}