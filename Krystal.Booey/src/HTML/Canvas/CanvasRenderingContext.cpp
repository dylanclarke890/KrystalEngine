#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"

namespace krys::boo::html
{
  CanvasRenderingContext::CanvasRenderingContext(CanvasBase &canvas, Type type) noexcept
      : _canvas(canvas), _type(type)
  {
  }

  bool CanvasRenderingContext::DelegatesDisplay() const noexcept
  {
    // FIXME: An accelerated 2D context delegates its display too, once there's one.
    return IsGPUBased();
  }
}
