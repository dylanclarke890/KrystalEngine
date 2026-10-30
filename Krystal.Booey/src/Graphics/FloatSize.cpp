#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const FloatSize &size) noexcept
  {
    return ts << "width=" << TextStream::FormatNumberRespectingIntegers(size.Width())
              << " height=" << TextStream::FormatNumberRespectingIntegers(size.Height());
  }
}
