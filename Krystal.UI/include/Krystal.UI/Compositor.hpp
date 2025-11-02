#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Document.hpp"

namespace Krys::UI
{
  class Compositor
  {
    NO_COPY_MOVE(Compositor)

  private:
    Gfx::IContext &_context;
    Gfx::IRenderer &_renderer;
    Gfx::CommandArena _commandArena;

    struct State
    {
      ElementHandle CurrentElement;
      float ParentX;
      float ParentY;
    };

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

      _commandArena.Reset();
      auto renderTargetDimensions = _context.RenderTargets().GetDimensions(renderTarget);
      document.Reflow(renderTargetDimensions.x, renderTargetDimensions.y);
      Gfx::CommandList commandList(_commandArena);
      commandList.Push(Gfx::BindRenderTargetCommand {renderTarget});
      RenderElement(commandList, document, {document.Body(), 0.f, 0.f});
      _renderer.Submit(commandList);
    }

  private:
    void RenderElement(Gfx::CommandList &list, Document &document, const State &state)
    {
      auto &element = document.Get(state.CurrentElement);
      auto node = element.LayoutNode;

      float x = state.ParentX + NodeLayoutGetLeft(node);
      float y = state.ParentY + NodeLayoutGetTop(node);
      float w = NodeLayoutGetWidth(node);
      float h = NodeLayoutGetHeight(node);

      Gfx::RectCommand command;
      command.Position = {x, y};
      command.Size = {w, h};
      command.BackgroundColour = NodeStyleGetBackgroundColour(node);
      command.BorderColour = NodeStyleGetBorderColour(node);
      list.Push(command);

      for (auto &child : element.Children)
      {
        RenderElement(list, document, {child, x, y});
      }
    }
  };
}