#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/CommandRecorder.hpp"
#include "Krystal.UI/Document.hpp"
#include "Krystal.UI/LayerBuilder.hpp"
#include "Krystal.Gfx/IRenderer.hpp"

namespace Krys::UI
{
  class Compositor
  {
    NO_COPY_MOVE(Compositor)

  private:
    LayerBuilder _builder;
    CommandRecorder _recorder;
    Gfx::IRenderer &_renderer;

  public:
    Compositor(Gfx::IRenderer &renderer) noexcept : _renderer(renderer)
    {
    }

    ~Compositor() = default;

    void Render(Document &document, float maxWidth, float maxHeight)
    {
      document.Layout(maxWidth, maxHeight);
      List<Layer> layers = _builder.Build(document);
      _recorder.Record(layers, document);

      for (auto &layer : layers)
      {
        for (auto &cmdList : layer.CommandLists)
        {
          _renderer.Submit(cmdList);
        }
      }
    }
  };
}