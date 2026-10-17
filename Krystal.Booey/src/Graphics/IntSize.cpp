#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  IntSize::IntSize(const FloatSize &size) noexcept
      : _width(ClampToInteger(size.Width())), _height(ClampToInteger(size.Height()))
  {
  }

  TextStream &operator<<(TextStream &ts, const IntSize &size) noexcept
  {
    return ts << "width=" << size.Width() << " height=" << size.Height();
  }
}
