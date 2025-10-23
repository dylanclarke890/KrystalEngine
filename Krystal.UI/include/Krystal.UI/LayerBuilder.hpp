#pragma once

#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.UI/Document.hpp"
#include "Krystal.UI/Layer.hpp"

namespace Krys::UI
{
  class LayerBuilder
  {
    NO_COPY_MOVE(LayerBuilder)

  public:
    LayerBuilder() noexcept = default;

    ~LayerBuilder() = default;

    NO_DISCARD List<Layer> Build(const Document &document) noexcept
    {
      List<Layer> layers;
      auto &body = document.GetBody();
      ComputedBounds cb = body.GetComputedBounds();
      auto &item = layers.emplace_back(
        Layer {body.GetHandle(), cb.Width, cb.Height, Gfx::CommandArena {}, List<Gfx::CommandList> {}});
      return layers;
    }
  };
}