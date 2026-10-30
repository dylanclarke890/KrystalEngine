#include "Krystal.Booey/Graphics/DoubleSize.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"

namespace krys::boo
{
  DoubleSize::DoubleSize(const FloatSize &size) noexcept : _width(size.Width()), _height(size.Height())
  {
  }
}
