#include "Krystal.Booey/Graphics/Geometry/DoubleSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"

namespace krys::boo
{
  DoubleSize::DoubleSize(const FloatSize &size) noexcept : _width(size.Width()), _height(size.Height())
  {
  }
}
