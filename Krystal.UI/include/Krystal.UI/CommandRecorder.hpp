#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.UI/Document.hpp"
#include "Krystal.UI/Element.hpp"
#include "Krystal.UI/Layer.hpp"

namespace Krys::UI
{
  class CommandRecorder
  {
    NO_COPY_MOVE(CommandRecorder)

  public:
    CommandRecorder() = default;

    ~CommandRecorder() = default;

    NO_DISCARD void Record(List<Layer> &layers, UI::Document &document)
    {
      for (auto &layer : layers)
      {
        auto &commandList = layer.CommandLists.emplace_back(layer.CommandArena);
        auto &root = document.GetByHandle(layer.RootElement);
        Record(layer, document, root, commandList);
      }
    }

  private:
    void Record(Layer &layer, UI::Document &document, UI::Element &element, Gfx::CommandList &cmdList,
                const Maths::Vec2 &parentOffset = {0.f, 0.f})
    {
      UI::ComputedBounds cb = element.GetComputedBounds();

      // Compute absolute position in root space
      float absX = parentOffset.x + cb.X;
      float absY = parentOffset.y + cb.Y;

      cmdList.Push<Gfx::RectCommand>({
        .BackgroundColour = element.GetBackgroundColor(),
        .BorderColour = element.GetBorderColor(),
        .Position = {absX, absY},
        .Size = {cb.Width, cb.Height},
        .BorderThickness = element.GetBorderWidth(),
        .BorderRadius = element.GetBorderRadius(),
      });

      // Recurse into children
      for (const auto &childHandle : element.GetChildren())
      {
        auto &childElement = document.GetByHandle(childHandle);
        Record(layer, document, childElement, cmdList, {absX, absY});
      }
    }
  };
}