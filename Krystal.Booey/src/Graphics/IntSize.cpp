#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo
{
  IntSize::IntSize(const FloatSize &size) noexcept
      : _width(ClampToInteger(size.Width())), _height(ClampToInteger(size.Height()))
  {
  }
}
