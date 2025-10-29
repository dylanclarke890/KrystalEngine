#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
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

  public:
    Compositor(Gfx::IContext &context, Gfx::IRenderer &renderer) noexcept
        : _context(context), _renderer(renderer)
    {
    }

    ~Compositor() = default;

    void Render(Document &document, float maxWidth, float maxHeight) noexcept
    {
      _commandArena.Reset();
      document.Reflow(maxWidth, maxHeight);
      Gfx::CommandList commandList(_commandArena);

      for (auto it = document.BeginBreadthFirst(); it != document.EndBreadthFirst(); ++it)
      {
        auto &element = document.Get(*it);

        auto node = element.LayoutNode;
        float x = NodeLayoutGetLeft(node);
        float y = NodeLayoutGetTop(node);
        float width = NodeLayoutGetWidth(node);
        float height = NodeLayoutGetHeight(node);

        Gfx::RectCommand rectCmd;
        rectCmd.Position = {x, y};
        rectCmd.Size = {width, height};
        rectCmd.BackgroundColour = NodeStyleGetBackgroundColour(node);
        rectCmd.BorderColour = NodeStyleGetBorderColour(node);
        rectCmd.BorderThickness = NodeLayoutGetBorder(node, Edge::Left);
        rectCmd.BorderRadius = 0.0f;
        commandList.Push(rectCmd);
      }

      _renderer.Submit(commandList);
    }

  private:
  };
}