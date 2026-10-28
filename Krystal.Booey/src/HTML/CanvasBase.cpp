#include "Krystal.Booey/HTML/CanvasBase.hpp"

namespace krys::boo::html
{
  CanvasBase::CanvasBase(IntSize size) noexcept : _size(size)
  {
  }

  void CanvasBase::DidDraw(const Maybe<FloatRect> &) noexcept
  {
  }

  void CanvasBase::SetSize(const IntSize &size) noexcept
  {
    _size = size;
  }
}
