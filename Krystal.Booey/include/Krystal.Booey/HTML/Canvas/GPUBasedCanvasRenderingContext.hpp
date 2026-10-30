#pragma once

#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"

namespace krys::boo::html
{
  class HTMLCanvasElement;

  /// @brief A context that draws on the GPU, into a surface its canvas's layer shows.
  class GPUBasedCanvasRenderingContext : public CanvasRenderingContext
  {
  public:
    /// @brief Called when the canvas's size changes, to resize the drawing surface.
    virtual void Reshape() noexcept = 0;

  protected:
    GPUBasedCanvasRenderingContext(CanvasBase &canvas, Type type) noexcept;

    KRYS_NODISCARD HTMLCanvasElement *HtmlCanvas() const noexcept;

    /// @brief Reports that the whole canvas was drawn.
    void MarkCanvasChanged() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CANVAS_RENDERING_CONTEXT(krys::boo::html::GPUBasedCanvasRenderingContext,
                                                     IsGPUBased())
