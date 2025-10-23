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
    void Record(Layer &layer, UI::Document &document, UI::Element &element, Gfx::CommandList &cmdList)
    {
      UI::ComputedBounds cb = element.GetComputedBounds();
      // TODO: this needs to be configurable per backend or handled elsewhere
      cb.Y = document.GetByHandle(layer.RootElement).GetComputedBounds().Height - cb.Y - cb.Height;
      cmdList.Push<Gfx::RectCommand>({
        .BackgroundColour = element.GetBackgroundColor(),
        .BorderColour = element.GetBorderColor(),
        .Position = {cb.X, cb.Y},
        .Size = {cb.Width, cb.Height},
        .BorderThickness = element.GetBorderWidth(),
        .BorderRadius = element.GetBorderRadius(),
      });

      for (const auto &childHandle : element.GetChildren())
      {
        auto &childElement = document.GetByHandle(childHandle);
        Record(layer, document, childElement, cmdList);
      }
    }
  };
}