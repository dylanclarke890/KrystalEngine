#include "Krystal.Booey/HTML/Canvas/GPUBasedCanvasRenderingContext.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"

namespace krys::boo::html
{
  GPUBasedCanvasRenderingContext::GPUBasedCanvasRenderingContext(CanvasBase &canvas, Type type) noexcept
      : CanvasRenderingContext(canvas, type)
  {
    krys_debug_assert(IsGPUBased());
  }

  HTMLCanvasElement *GPUBasedCanvasRenderingContext::HtmlCanvas() const noexcept
  {
    return DynamicDowncast<HTMLCanvasElement>(GetCanvasBase());
  }

  void GPUBasedCanvasRenderingContext::MarkCanvasChanged() noexcept
  {
    auto &canvas = GetCanvasBase();
    canvas.DidDraw(FloatRect {{}, FloatSize(canvas.Size())});
  }
}
